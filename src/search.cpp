/*
 * File: search.cpp
 * Author: <contact@simshadows.com>
 */

//#include <cstdlib>
#include <assert.h>
#include <chrono>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <tuple>

#include "mhwi_build_search.h"
#include "core/core.h"
#include "database/database.h"
#include "support/support.h"
#include "utils/utils.h"
#include "utils/utils_strings.h"
#include "utils/logging.h"
#include "utils/pruning_vector.h"
#include "utils/counter.h"
#include "utils/counter_subset_seen_map.h"


namespace MHWIBuildSearch
{


using SSBTuple = std::tuple<SkillMap, SetBonusMap>;


template<class StoredData>
using SSBSeenMap = Utils::CounterSubsetSeenMap<StoredData, SkillMap, SetBonusMap>;


struct DecoCombo {
    std::vector<const Decoration*> decos;
    SkillMap skills_including_previous;
};


struct ArmourPieceCombo {
    const ArmourPiece* armour_piece;
    DecoEquips decos;
};


struct ArmourSetCombo {
    ArmourEquips armour;
    DecoEquips decos;
};


struct WeaponInstanceExtended {
    WeaponInstance     instance;
    WeaponContribution contributions;
    double             ceiling_efr;
};



struct WeaponInstancePruneFn {
    // Return true if left can prune away right.
    // Left equalling right can also prune away right.
    bool operator()(const std::pair<WeaponInstance, WeaponContribution>& left,
                    const std::pair<WeaponInstance, WeaponContribution>& right ) const noexcept {
        const WeaponContribution& lc = left.second;
        const WeaponContribution& rc = right.second;

        if ((lc.weapon_raw < rc.weapon_raw) || (lc.weapon_aff < rc.weapon_aff)) {
            return false;
        }

        {
            assert(std::is_sorted(lc.deco_slots.begin(), lc.deco_slots.end(), std::greater<unsigned int>()));
            assert(std::is_sorted(rc.deco_slots.begin(), rc.deco_slots.end(), std::greater<unsigned int>()));

            // Left cannot replace right if left has less deco slots.
            if (lc.deco_slots.size() < rc.deco_slots.size()){
                return false;
            }

            // We now know left has at least the same number of deco slots as right.
            // Now, left cannot replace right if some some set of decos that fit in right
            // cannot fit in left.
            auto l_head = lc.deco_slots.begin();
            for (const unsigned int rv : rc.deco_slots) {
                const unsigned int lv = *l_head;
                if (lv < rv) {
                    return false;
                }
                ++l_head; // TODO: Increase safety?
            }
        }

        /*
         * We prune skills and set bonuses by this truth table:
         *
         *                | rc=None  | rc=setbonusA | rc=setbonusB
         *   -------------|----------|--------------|--------------
         *   lc=None      | continue | return False | return False
         *   lc=setbonusA | continue | continue     | return False
         *   lc=setbonusB | continue | return False | continue
         *   -------------|----------|--------------|--------------
         */
        if ((rc.set_bonus && (lc.set_bonus != rc.set_bonus)) || (rc.skill && (lc.skill != rc.skill))) {
            return false;
        }

        if (rc.health_regen_active && !lc.health_regen_active) {
            return false;
        }

        if ((!lc.is_constant_sharpness) && rc.is_constant_sharpness) {
            // TODO: Returning constant false is conservative. Try apply 0 handicraft?
            return false;
        } else {
            return SharpnessGauge::left_has_eq_or_more_hits(lc.maximum_sharpness, rc.maximum_sharpness);
        }
    }
};


static std::vector<WeaponInstanceExtended> prepare_weapons(const Database& db,
                                                           const SearchParameters& params,
                                                           std::unordered_set<const SetBonus*>& set_bonus_subset) {

    auto start_t = std::chrono::steady_clock::now();

    std::vector<const Weapon*> weapons = db.weapons.get_all_of_weaponclass(params.weapon_class);
    Utils::log_stat("Weapons: ", weapons.size());
    assert(weapons.size());

    std::vector<std::pair<WeaponInstance, WeaponContribution>> unpruned;

    for (const Weapon * const weapon : weapons) {
        const auto augment_instances = WeaponAugmentsInstance::generate_maximized_instances(weapon);
        const auto upgrade_instances = WeaponUpgradesInstance::generate_maximized_instances(weapon);
        for (const std::shared_ptr<WeaponAugmentsInstance>& a : augment_instances) {
            for (const std::shared_ptr<WeaponUpgradesInstance>& u : upgrade_instances) {
                WeaponInstance new_inst = {weapon, a, u};
                WeaponContribution new_cont = new_inst.calculate_contribution(db);
                if (params.health_regen_required && !new_cont.health_regen_active) {
                    continue;
                }
                if (!Utils::set_has_key(set_bonus_subset, new_cont.set_bonus)) {
                    new_cont.set_bonus = nullptr;
                }
                if (!params.skill_spec.is_in_subset(new_cont.skill)) {
                    new_cont.skill = nullptr;
                }
                unpruned.emplace_back(std::move(new_inst), std::move(new_cont));
            }
        }
    }
    const std::size_t stat_pre = unpruned.size();

    Utils::PruningVector<std::pair<WeaponInstance, WeaponContribution>, WeaponInstancePruneFn> pruned;

    //std::size_t i = 0;
    for (auto& e : unpruned) {
        //std::clog << i++ << "/" << std::to_string(stat_pre) << "\n";
        pruned.try_push_back(std::move(e));
    }

    SkillMap maximized_skills;
    for (const auto& e : params.skill_spec) {
        maximized_skills.set(e.first, e.first->secret_limit);
    }

    std::vector<WeaponInstanceExtended> ret;
    for (const auto& original : pruned.underlying()) {
        const double ceiling_efr = calculate_efr_from_skills_lookup(db, original.second, maximized_skills, params.skill_spec);
        ret.push_back({std::move(original.first), std::move(original.second), ceiling_efr});
    }

    Utils::log_stat_reduction("Generated weapon augment+upgrade instances: ", stat_pre, ret.size());
    Utils::log_stat_duration("  >>> weapon augment+upgrade instances: ", start_t);
    Utils::log_stat();

    return ret;
}


static std::array<std::vector<const Decoration*>, k_MAX_DECO_SIZE> prepare_decos(const Database& db,
                                                                                 const SkillSpec& skill_spec) {
    std::vector<const Decoration*> all_decos = db.decos.get_all();

    std::unordered_map<const Skill*, const Decoration*> simplest_decos;
    std::vector<const Decoration*> decos_to_keep;

    // We first put decos that only contribute one level (within the skill spec) to the build in simplest_decos,
    // and any deco that contributes multiple levels to decos_to_keep.
    // (This isn't perfect since this allows all size-4 single-skill decos with skill levels 2 or greater to all be included,
    // regardless of the fact that we ideally want just the bigger deco.)
    // TODO: Make a better algorithm!
    for (const Decoration * const deco : all_decos) {
        const Skill* found_skill;
        std::size_t num_contribution = 0;
        for (const auto& e : deco->skills) {
            if (skill_spec.is_in_subset(e.first)) {
                found_skill = e.first;
                num_contribution += e.second;
            }
        }
        if (num_contribution == 1) {
            if (Utils::map_has_key(simplest_decos, found_skill)
                    && (simplest_decos.at(found_skill)->slot_size < deco->slot_size)) {
                continue;
            }
            simplest_decos[found_skill] = deco;
        } else if (num_contribution > 1) {
            decos_to_keep.push_back(deco);
        }
    }
    // Now, we merge simplest_decos into decos_to_keep.
    for (const auto& e : simplest_decos) {
        decos_to_keep.push_back(e.second);
    }

    // Sort
    const auto cmp = [](const Decoration * const a, const Decoration * const b){
        return a->slot_size > b->slot_size;
    };
    std::sort(decos_to_keep.begin(), decos_to_keep.end(), cmp);

    Utils::log_stat_reduction("Decorations pruning: ", all_decos.size(), decos_to_keep.size());

    std::array<std::vector<const Decoration*>, k_MAX_DECO_SIZE> ret;
    for (const Decoration* deco : decos_to_keep) {
        assert(deco->slot_size >= 1);
        static_assert(k_MIN_DECO_SIZE == 1, "Assumption violation.");
        for (std::size_t i = deco->slot_size - 1; i < ret.size(); ++i) {
            ret[i].emplace_back(deco);
        }
    }
    return ret;
}


static std::vector<const Charm*> prepare_charms(const Database& db, const SkillSpec& skill_spec) {
    std::vector<const Charm*> ret = db.charms.get_all();

    const std::size_t stat_pre = ret.size();

    const auto pred = [&](const Charm* x){
        for (const Skill* s : x->skills) {
            if (skill_spec.is_in_subset(s)) return false;
        }
        return true;
    };
    ret.erase(std::remove_if(ret.begin(), ret.end(), pred), ret.end());

    const std::size_t stat_post = ret.size();

    Utils::log_stat_reduction("Charms filtering:    ", stat_pre, stat_post);

    return ret;
}


static std::map<ArmourSlot, std::vector<const ArmourPiece*>> prepare_armour(const Database& db,
                                                                            const SearchParameters& params) {
    std::map<ArmourSlot, std::vector<const ArmourPiece*>> ret = db.armour.get_all_pieces_by_slot();

    std::size_t head_pre  = ret.at(ArmourSlot::head).size();
    std::size_t chest_pre = ret.at(ArmourSlot::chest).size();
    std::size_t arms_pre  = ret.at(ArmourSlot::arms).size();
    std::size_t waist_pre = ret.at(ArmourSlot::waist).size();
    std::size_t legs_pre  = ret.at(ArmourSlot::legs).size();

    assert(ret.size() == 5);
    assert(head_pre);
    assert(chest_pre);
    assert(arms_pre);
    assert(arms_pre);
    assert(legs_pre);

    const auto pred = [&](const ArmourPiece* x){
        switch (armour_variant_to_tier(x->variant)) {
            case Tier::low_rank:    return !params.allow_low_rank;
            case Tier::high_rank:   return !params.allow_high_rank;
            case Tier::master_rank: return !params.allow_master_rank;
            default:
                throw std::runtime_error("Invalid tier.");
        }
    };

    for (auto& e : ret) {
        auto& a = e.second;
        a.erase(std::remove_if(a.begin(), a.end(), pred), a.end());
    }

    Utils::log_stat_reduction("Head piece  - filtering by tier: ", head_pre,  ret.at(ArmourSlot::head).size());
    Utils::log_stat_reduction("Chest piece - filtering by tier: ", chest_pre, ret.at(ArmourSlot::chest).size());
    Utils::log_stat_reduction("Arm piece   - filtering by tier: ", arms_pre,  ret.at(ArmourSlot::arms).size());
    Utils::log_stat_reduction("Waist piece - filtering by tier: ", waist_pre, ret.at(ArmourSlot::waist).size());
    Utils::log_stat_reduction("Leg piece   - filtering by tier: ", legs_pre,  ret.at(ArmourSlot::legs).size());

    return ret;
}


static std::vector<DecoCombo> generate_deco_combos(const std::vector<unsigned int>& deco_slots,
                                                   const std::array<std::vector<const Decoration*>,
                                                                    k_MAX_DECO_SIZE>& sorted_decos,
                                                   const SkillSpec& skill_spec,
                                                   const SkillMap& existing_skills) {
    assert(std::is_sorted(deco_slots.begin(), deco_slots.end(), std::greater<unsigned int>()));

    if (!deco_slots.size()) return {{{}, existing_skills}};
    // The rest of the algorithm will assume the presence of decoration slots.

    // We will "consume" deco slots from deco_slots by tracking the first "unconsumed" slot.
    using DecoSlotsHead = std::vector<unsigned int>::const_iterator;
    using WorkingCombo = std::pair<DecoCombo, DecoSlotsHead>;

    // Start with a seed combo
    std::vector<WorkingCombo> incomplete_combos = {{{{}, existing_skills}, deco_slots.begin()}};
    std::vector<DecoCombo> complete_combos;

    const std::size_t sublist_index = deco_slots.front() - 1;
    assert(sublist_index < sorted_decos.size());
    const std::vector<const Decoration*>& deco_sublist = sorted_decos[sublist_index];

    for (const Decoration * const deco : deco_sublist) {

        // We copy since we want to add zero of this deco to them anyway.
        std::vector<WorkingCombo> new_incomplete_combos = incomplete_combos;

        for (const std::pair<DecoCombo, DecoSlotsHead>& combo : incomplete_combos) {
            assert(combo.second != deco_slots.end());

            const std::vector<const Decoration*>& curr_decos = combo.first.decos;
            const SkillMap& curr_skills = combo.first.skills_including_previous;
            const DecoSlotsHead& curr_head = combo.second;

            const unsigned int max_to_add = [&](){
                unsigned int x;
                for (const auto& e : deco->skills) {
                    if (skill_spec.is_in_subset(e.first)) {
                        assert(e.first->secret_limit >= curr_skills.get(e.first));
                        const unsigned int v = Utils::ceil_div(e.first->secret_limit - curr_skills.get(e.first), e.second);
                        if (v > x) x = v;
                    }
                }
                return x;
            }();

            //// TODO: Add this?
            //if (!max_to_add) continue;

            std::vector<const Decoration*> new_decos = curr_decos;
            SkillMap new_skills = curr_skills;
            DecoSlotsHead new_head = curr_head;

            // We skip 0 because we already copied all previous combos.
            for (unsigned int to_add = 1; to_add <= max_to_add; ++to_add) {
                if (*new_head < deco->slot_size) break; // Stop adding. Deco can no longer fit.

                new_decos.emplace_back(deco);
                new_skills.add_skills_filtered(*deco, skill_spec);
                ++new_head;

                if (new_head == deco_slots.end()) {
                    complete_combos.push_back({new_decos, new_skills});
                    // TODO: ugh, why does emplace_back not work?
                    break;
                } else {
                    new_incomplete_combos.push_back({{new_decos, new_skills}, new_head});
                    // TODO: ugh, why does emplace_back not work?
                }
            }

        }

        incomplete_combos = std::move(new_incomplete_combos);
    }

    for (WorkingCombo& combo : incomplete_combos) {
        complete_combos.emplace_back(std::move(combo.first));
    }

    return complete_combos;
}


//static std::vector<DecoCombo> generate_deco_combos(const std::vector<unsigned int>& deco_slots,
//                                                   const std::array<std::vector<const Decoration*>,
//                                                                    k_MAX_DECO_SIZE>& sorted_decos,
//                                                   const SkillSpec& skill_spec,
//                                                   const SkillMap& existing_skills) {
//    assert(std::is_sorted(deco_slots.begin(), deco_slots.end(), std::greater<unsigned int>()));
//
//    if (!deco_slots.size()) return {{{}, existing_skills}};
//    // The rest of the algorithm will assume the presence of decoration slots.
//
//    // We will "consume" deco slots from deco_slots by tracking the first "unconsumed" slot.
//    using DecoSlotsHead = std::vector<unsigned int>::const_iterator;
//
//    using WorkingCombo  = std::pair<std::vector<const Decoration*>, DecoSlotsHead>;
//    using WorkingList   = std::vector<WorkingCombo>;
//    using CompleteCombo = std::vector<const Decoration*>;
//    using CompleteList  = std::vector<CompleteCombo>;
//
//    WorkingList incomplete_combos = {{{}, deco_slots.begin()}}; // Start with a seed combo
//    CompleteList complete_combos;
//
//    const std::size_t sublist_index = deco_slots.front() - 1;
//    assert(sublist_index < sorted_decos.size());
//    const std::vector<const Decoration*>& deco_sublist = sorted_decos[sublist_index];
//
//    for (const Decoration* deco : deco_sublist) {
//
//        // We copy-initialize since we want to add zero of this deco to them anyway.
//        WorkingList new_incomplete_combos = incomplete_combos;
//
//        unsigned int max_to_add = 0;
//        for (const auto& e : deco->skills) {
//            if (skill_spec.is_in_subset(e.first)) {
//                assert(e.first->secret_limit >= existing_skills.get(e.first));
//                const unsigned int v = Utils::ceil_div(e.first->secret_limit - existing_skills.get(e.first), e.second);
//                if (v > max_to_add) max_to_add = v;
//            }
//        }
//
//        for (WorkingCombo combo : incomplete_combos) {
//            assert(combo.second != deco_slots.end());
//
//            // We skip 0 because we already copied all previous combos.
//            std::vector<const Decoration*> curr_decos = combo.first;
//            DecoSlotsHead curr_head = combo.second;
//            for (unsigned int to_add = 1; to_add <= max_to_add; ++to_add) {
//                if (*curr_head < deco->slot_size) break; // Stop adding. Deco can no longer fit.
//
//                curr_decos.emplace_back(deco);
//                ++curr_head;
//
//                if (curr_head == deco_slots.end()) {
//                    complete_combos.emplace_back(curr_decos);
//                    break;
//                } else {
//                    new_incomplete_combos.emplace_back(curr_decos, curr_head);
//                }
//            }
//
//        }
//
//        incomplete_combos = std::move(new_incomplete_combos);
//    }
//
//    std::vector<DecoCombo> final_combos;
//    for (CompleteCombo& combo : complete_combos) {
//        SkillMap skills = existing_skills;
//        skills.add_skills_filtered(combo, skill_spec);
//        final_combos.push_back({std::move(combo), std::move(skills)});
//    }
//    for (WorkingCombo& combo : incomplete_combos) {
//        SkillMap skills = existing_skills;
//        skills.add_skills_filtered(combo.first, skill_spec);
//        final_combos.push_back({std::move(combo.first), std::move(skills)});
//    }
//
//    return final_combos;
//}


static SSBSeenMap<ArmourPieceCombo> generate_slot_combos(const std::vector<const ArmourPiece*>& pieces,
                                                          const std::array<std::vector<const Decoration*>,
                                                                           k_MAX_DECO_SIZE>& sorted_decos,
                                                          const SkillSpec& skill_spec,
                                                          const std::unordered_set<const SetBonus*>& set_bonus_subset,
                                                          const std::string& debug_msg) {
    // We will assume decorations are sorted.
    // TODO: Add a runtime assert.

    SSBSeenMap<ArmourPieceCombo> seen_set;

    unsigned long long stat_pre = 0;

    for (const ArmourPiece* piece : pieces) {

        SkillMap armour_skills;
        armour_skills.add_skills_filtered(*piece, skill_spec);

        std::vector<DecoCombo> deco_combos = generate_deco_combos(piece->deco_slots,
                                                                  sorted_decos,
                                                                  skill_spec,
                                                                  armour_skills);

        stat_pre += deco_combos.size(); // TODO: How do I know this won't overflow?

        for (DecoCombo& deco_combo : deco_combos) {

            SSBTuple ssb = {std::move(deco_combo.skills_including_previous), {}};
            if (Utils::set_has_key(set_bonus_subset, piece->set_bonus)) {
                std::get<1>(ssb).set(piece->set_bonus, 1);
            }

            seen_set.add({piece, std::move(deco_combo.decos)}, std::move(ssb));
        }
    }

    Utils::log_stat_reduction(debug_msg, stat_pre, seen_set.size());

    return seen_set;
}


static void merge_in_armour_list(SSBSeenMap<ArmourSetCombo>& armour_combos,
                                 const SSBSeenMap<ArmourPieceCombo>& piece_combos,
                                 const SkillSpec& skill_spec) {
    auto prev_armour_combos = armour_combos.get_data_as_vector();

    for (const auto& e1 : prev_armour_combos) {
        const SSBTuple&       set_combo_ssb = e1.first;
        const ArmourSetCombo& set_combo     = e1.second;

        assert(std::get<0>(set_combo_ssb).only_contains_skills_in_spec(skill_spec));(void)skill_spec;

        for (const auto& e2 : piece_combos) {
            const SSBTuple&         piece_combo_ssb = e2.first;
            const ArmourPieceCombo& piece_combo     = e2.second;

            const auto op1 = [&](){
                ArmourSetCombo x = set_combo;
                x.armour.add(piece_combo.armour_piece);
                x.decos.merge_in(piece_combo.decos);
                assert(x.decos.fits_in(x.armour, {}));
                return x;
            };

            const auto op2 = [&](){
                SSBTuple x = set_combo_ssb;
                std::get<0>(x).merge_in(std::get<0>(piece_combo_ssb));
                std::get<1>(x).merge_in(std::get<1>(piece_combo_ssb));
                assert(std::get<0>(x).only_contains_skills_in_spec(skill_spec));(void)skill_spec;
                return x;
            };

            armour_combos.add(op1(), op2());
        }
    }
}


static void refilter_weapons(std::vector<WeaponInstanceExtended>& weapons,
                             const double max_efr,
                             const std::size_t original_weapon_count) {
    const auto pred = [&](const WeaponInstanceExtended& x){
        return x.ceiling_efr <= max_efr;
    };
    weapons.erase(std::remove_if(weapons.begin(), weapons.end(), pred), weapons.end());

    Utils::log_stat_reduction("\n\nRepruned weapons with EFR " + std::to_string(max_efr) + ": ",
                              original_weapon_count,
                              weapons.size());
}


static void do_search(const Database& db, const SearchParameters& params) {

    auto total_start_t = std::chrono::steady_clock::now();

    std::clog << params.skill_spec.get_humanreadable() << std::endl << std::endl;

    std::unordered_set<const SetBonus*> set_bonus_subset;

    {
        const std::vector<const SetBonus*> all_set_bonuses = db.skills.get_all_set_bonuses();

        for (const SetBonus * const set_bonus : all_set_bonuses) {
            for (const auto& e : set_bonus->stages) {
                if (params.skill_spec.is_in_subset(e.second)) {
                    set_bonus_subset.emplace(set_bonus);
                    break;
                }
            }
        }

        if (set_bonus_subset.size()) {
            std::clog << "Set bonuses to be considered:";
            for (const SetBonus * const set_bonus : set_bonus_subset) {
                std::clog << std::endl << "  " << set_bonus->name;
            }
            std::clog << std::endl << std::endl;
        }
    }

    std::vector<WeaponInstanceExtended> weapons = prepare_weapons(db, params, set_bonus_subset);
    const std::size_t weapons_initial_size = weapons.size();
    assert(weapons_initial_size);

    auto start_t = std::chrono::steady_clock::now();

    std::array<std::vector<const Decoration*>, k_MAX_DECO_SIZE> grouped_sorted_decos = prepare_decos(db, params.skill_spec);
    assert(grouped_sorted_decos.size() == 4);
    assert(grouped_sorted_decos[0].size());
    assert(grouped_sorted_decos[1].size());
    assert(grouped_sorted_decos[2].size());
    assert(grouped_sorted_decos[3].size());

    std::vector<const Charm*> charms = prepare_charms(db, params.skill_spec);
    assert(charms.size());

    std::map<ArmourSlot, std::vector<const ArmourPiece*>> armour = prepare_armour(db, params);

    assert(armour.size() == 5);
    assert(armour.at(ArmourSlot::head).size());
    assert(armour.at(ArmourSlot::chest).size());
    assert(armour.at(ArmourSlot::arms).size());
    assert(armour.at(ArmourSlot::waist).size());
    assert(armour.at(ArmourSlot::legs).size());

    SSBSeenMap<ArmourPieceCombo> head_combos = generate_slot_combos(armour.at(ArmourSlot::head),
                                                                    grouped_sorted_decos,
                                                                    params.skill_spec,
                                                                    set_bonus_subset,
                                                                    "Generated head+deco  combinations: ");
    SSBSeenMap<ArmourPieceCombo> chest_combos = generate_slot_combos(armour.at(ArmourSlot::chest),
                                                                     grouped_sorted_decos,
                                                                     params.skill_spec,
                                                                     set_bonus_subset,
                                                                     "Generated chest+deco combinations: ");
    SSBSeenMap<ArmourPieceCombo> arms_combos = generate_slot_combos(armour.at(ArmourSlot::arms),
                                                                    grouped_sorted_decos,
                                                                    params.skill_spec,
                                                                    set_bonus_subset,
                                                                    "Generated arms+deco  combinations: ");
    SSBSeenMap<ArmourPieceCombo> waist_combos = generate_slot_combos(armour.at(ArmourSlot::waist),
                                                                     grouped_sorted_decos,
                                                                     params.skill_spec,
                                                                     set_bonus_subset,
                                                                     "Generated waist+deco combinations: ");
    SSBSeenMap<ArmourPieceCombo> legs_combos = generate_slot_combos(armour.at(ArmourSlot::legs),
                                                                    grouped_sorted_decos,
                                                                    params.skill_spec,
                                                                    set_bonus_subset,
                                                                    "Generated legs+deco  combinations: ");
    Utils::log_stat_duration("  >>> decos, charms, and armour slot combos: ", start_t);
    Utils::log_stat();

    // We build the initial build list.
    
    SSBSeenMap<ArmourSetCombo> armour_combos;
    for (const Charm* charm : charms) {
        ArmourSetCombo combo;
        SSBTuple ssb;

        combo.armour.add(charm);
        std::get<0>(ssb).add_skills_filtered(*charm, charm->max_charm_lvl, params.skill_spec);
        assert(std::get<0>(ssb).only_contains_skills_in_spec(params.skill_spec));

        armour_combos.add(std::move(combo), std::move(ssb));
    }

    // And now, we merge in our slot combinations!

    start_t = std::chrono::steady_clock::now();
    unsigned long long stat_pre = armour_combos.size() * head_combos.size();
    //
    merge_in_armour_list(armour_combos, head_combos, params.skill_spec);
    //
    Utils::log_stat_reduction("Merged in head+deco  combinations: ", stat_pre, armour_combos.size());
    Utils::log_stat_duration("  >>> head combo merge: ", start_t);

    //std::vector<std::string> buf;
    //for (const auto& e : head_combos) {
    //    buf.emplace_back(std::get<0>(e.first).get_humanreadable()
    //                     + "\n"
    //                     + std::to_string(std::get<1>(e.first).size()) );
    //}
    //std::sort(buf.begin(), buf.end());
    //std::clog << Utils::str_join(buf.begin(), buf.end(), "\n\n") << "\n\n";

    start_t = std::chrono::steady_clock::now();
    stat_pre = armour_combos.size() * chest_combos.size();
    //
    merge_in_armour_list(armour_combos, chest_combos, params.skill_spec);
    //
    Utils::log_stat_reduction("Merged in chest+deco combinations: ", stat_pre, armour_combos.size());
    Utils::log_stat_duration("  >>> chest combo merge: ", start_t);

    start_t = std::chrono::steady_clock::now();
    stat_pre = armour_combos.size() * arms_combos.size();
    //
    merge_in_armour_list(armour_combos, arms_combos, params.skill_spec);
    //
    Utils::log_stat_reduction("Merged in arms+deco  combinations: ", stat_pre, armour_combos.size());
    Utils::log_stat_duration("  >>> arms combo merge: ", start_t);

    start_t = std::chrono::steady_clock::now();
    stat_pre = armour_combos.size() * waist_combos.size();
    //
    merge_in_armour_list(armour_combos, waist_combos, params.skill_spec);
    //
    Utils::log_stat_reduction("Merged in waist+deco combinations: ", stat_pre, armour_combos.size());
    Utils::log_stat_duration("  >>> waist combo merge: ", start_t);

    start_t = std::chrono::steady_clock::now();
    stat_pre = armour_combos.size() * legs_combos.size();
    //
    merge_in_armour_list(armour_combos, legs_combos, params.skill_spec);
    //
    Utils::log_stat_reduction("Merged in legs+deco  combinations: ", stat_pre, armour_combos.size());
    Utils::log_stat_duration("  >>> legs combo merge: ", start_t);

    double best_efr = 0;

    //const std::size_t stat_pre2 = armour_combos.size();
    //std::size_t stat_progress2 = 0;
    std::size_t stat_wa_combos_explored = 0;
    std::size_t stat_wad_combos_explored = 0;
    start_t = std::chrono::steady_clock::now();

    for (const auto& e : armour_combos) {
        const SSBTuple&       ac_ssb = e.first;
        const ArmourSetCombo& ac     = e.second;

        bool reprune_weapons = false;
        for (const WeaponInstanceExtended& wc : weapons) {

            const SkillMap wac_skills = [&](){
                SkillMap x = std::get<0>(ac_ssb); // "Weapon-armour-combo"
                if (wc.contributions.skill) x.increment(wc.contributions.skill, 1);
                // TODO: Should we merge in set bonuses here?
                return x;
            }();

            const SetBonusMap wac_set_bonuses = [&](){
                SetBonusMap ret = ac.armour.get_set_bonuses();
                if (wc.contributions.set_bonus) ret.increment(wc.contributions.set_bonus, 1);
                return ret;
            }();
                

            std::vector<DecoCombo> w_deco_combos = generate_deco_combos(wc.contributions.deco_slots,
                                                                         grouped_sorted_decos,
                                                                         params.skill_spec,
                                                                         wac_skills);
            ++stat_wa_combos_explored;
            stat_wad_combos_explored += w_deco_combos.size();

            for (DecoCombo& dc : w_deco_combos) {

                SkillMap skills = dc.skills_including_previous;
                skills.add_set_bonuses(wac_set_bonuses);

                // Filter out anything that doesn't meet minimum requirements
                if (!params.skill_spec.skills_meet_minimum_requirements(skills)) continue;
                assert((!params.health_regen_required) || wc.contributions.health_regen_active);

                DecoEquips curr_decos = std::move(dc.decos);
                curr_decos.merge_in(ac.decos);
                assert(curr_decos.fits_in(ac.armour, wc.contributions));

                const double efr = calculate_efr_from_skills_lookup(db, wc.contributions, skills, params.skill_spec);

                if (efr > best_efr) {
                    best_efr = efr;

                    const std::string build_info = wc.instance.weapon->name + "\n\n"
                                                   + wc.instance.upgrades->get_humanreadable() + "\n\n"
                                                   + wc.instance.augments->get_humanreadable() + "\n\n"
                                                   + "Armour:\n"
                                                   + Utils::indent(ac.armour.get_humanreadable(), 4) + "\n\n"
                                                   + "Decorations:\n"
                                                   + Utils::indent(curr_decos.get_humanreadable(), 4) + "\n\n"
                                                   + "Skills:\n"
                                                   + Utils::indent(skills.get_humanreadable(), 4);
                    std::clog << "\n\nFound EFR: " + std::to_string(best_efr) + "\n\n"
                              << Utils::indent(build_info, 4) + "\n";
                    reprune_weapons = true;
                }

            }

        }
        if (reprune_weapons) refilter_weapons(weapons, best_efr, weapons_initial_size);

        //(void)stat_progress2;
        //(void)stat_pre2;
        //std::clog << std::to_string(stat_progress2) + "/" + std::to_string(stat_pre2) + "\n";
        //++stat_progress2;
    }

    Utils::log_stat_expansion("\nWeapon-armour --> +decos combinations explored: ",
                              stat_wa_combos_explored,
                              stat_wad_combos_explored);
    Utils::log_stat_duration("  >>> weapon combo merge: ", start_t);
    Utils::log_stat();

    std::clog << std::endl;
    Utils::log_stat_duration("Search execution time (before teardown): ", total_start_t);
    Utils::log_stat();

    //// A tempting cheat for skipping all the memory cleanup, saving us time.
    //// We could technically use it since we don't open any resources...
    //std::clog << std::flush;
    //std::exit(0);
}


void search_cmd(const std::string& search_parameters_path) {

    const Database db = Database::get_db();
    const SearchParameters params = read_file(db, search_parameters_path);

    do_search(db, params);
}


} // namespace

