/*
 * File: search.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <algorithm>
#include <iterator>
#include <iostream>

#include "mhwi_build_search.h"
#include "core/core.h"
#include "database/database.h"
#include "support/support.h"
#include "support_headeronly/skills_seen_set.h"
#include "utils/utils.h"


namespace MHWIBuildSearch
{


struct ArmourPieceCombo {
    const ArmourPiece* armour_piece;
    DecoEquips decos;

    SkillMap skills;
};


struct ArmourSetCombo {
    ArmourEquips armour;
    DecoEquips decos;

    SkillMap skills;

    ArmourSetCombo(const Charm * const charm)
        : armour (charm)
        , decos  {}
        , skills (armour.get_skills_without_set_bonuses())
    {
    }
};


static std::vector<const Weapon*> prepare_weapons(const Database& db,
                                                  const SearchParameters& params) {
    std::vector<const Weapon*> ret = db.weapons.get_all_of_weaponclass(params.weapon_class);
    assert(ret.size());

    Utils::log_stat("Weapons:             ", ret.size());

    return ret;
}


static std::array<std::vector<const Decoration*>, k_MAX_DECO_SIZE> prepare_decos(const Database& db,
                                                                                 const SkillSpec& skill_spec) {
    std::vector<const Decoration*> sorted_decos = db.decos.get_all();

    const std::size_t stat_pre = sorted_decos.size();

    // Filter
    const auto pred = [&](const Decoration* x){for (const auto& e : x->skills)
                                                   if (skill_spec.is_in_subset(e.first)) {
                                                       assert(e.second > 0);
                                                       return false;
                                                   }
                                               return true; };
    sorted_decos.erase(std::remove_if(sorted_decos.begin(), sorted_decos.end(), pred), sorted_decos.end());

    // Sort
    const auto cmp = [](const Decoration * const a, const Decoration * const b){return a->slot_size > b->slot_size;};
    std::sort(sorted_decos.begin(), sorted_decos.end(), cmp);

    const std::size_t stat_post = sorted_decos.size();

    Utils::log_stat_reduction("Decorations pruning: ", stat_pre, stat_post);

    std::array<std::vector<const Decoration*>, k_MAX_DECO_SIZE> ret;

    for (const Decoration* deco : sorted_decos) {
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


static std::map<ArmourSlot, std::vector<const ArmourPiece*>> get_pruned_armour(const Database& db,
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

    Utils::log_stat_reduction("Head pieces filtering by tier:  ", head_pre,  ret.at(ArmourSlot::head).size());
    Utils::log_stat_reduction("Chest pieces filtering by tier: ", chest_pre, ret.at(ArmourSlot::chest).size());
    Utils::log_stat_reduction("Arm pieces filtering by tier:   ", arms_pre,  ret.at(ArmourSlot::arms).size());
    Utils::log_stat_reduction("Waist pieces filtering by tier: ", waist_pre, ret.at(ArmourSlot::waist).size());
    Utils::log_stat_reduction("Leg pieces filtering by tier:   ", legs_pre,  ret.at(ArmourSlot::legs).size());
    Utils::log_stat();

    return ret;
}


static std::vector<std::vector<const Decoration*>> generate_deco_combos(const std::vector<unsigned int>& deco_slots,
                                                                        const std::array<std::vector<const Decoration*>,
                                                                                         k_MAX_DECO_SIZE>& sorted_decos,
                                                                        const SkillSpec skill_spec) {
    assert(std::is_sorted(deco_slots.begin(), deco_slots.end(), std::greater<unsigned int>()));

    if (!deco_slots.size()) return {};
    // The rest of the algorithm will assume the presence of decoration slots.

    // We will "consume" deco slots from deco_slots by tracking the first "unconsumed" slot.
    using DecoSlotsHead = std::vector<unsigned int>::const_iterator;

    using WorkingCombo  = std::pair<std::vector<const Decoration*>, DecoSlotsHead>;
    using WorkingList   = std::vector<WorkingCombo>;
    using CompleteCombo = std::vector<const Decoration*>;
    using CompleteList  = std::vector<CompleteCombo>;

    WorkingList incomplete_combos = {{{}, deco_slots.begin()}}; // Start with a seed combo
    CompleteList complete_combos;

    const std::size_t sublist_index = deco_slots.front() - 1;
    assert(sublist_index < sorted_decos.size());
    const std::vector<const Decoration*>& deco_sublist = sorted_decos[sublist_index];

    for (const Decoration* deco : deco_sublist) {

        // We copy-initialize since we want to add zero of this deco to them anyway.
        WorkingList new_incomplete_combos = incomplete_combos;

        unsigned int max_to_add = 0;
        for (const auto& e : deco->skills) {
            if (skill_spec.is_in_subset(e.first)) {
                const unsigned int v = Utils::ceil_div(e.first->secret_limit, e.second);
                if (v > max_to_add) max_to_add = v;
            }
        }

        for (WorkingCombo combo : incomplete_combos) {
            assert(combo.second != deco_slots.end());

            // We skip 0 because we already copied all previous combos.
            std::vector<const Decoration*> curr_decos = combo.first;
            DecoSlotsHead curr_head = combo.second;
            for (unsigned int to_add = 1; to_add <= max_to_add; ++to_add) {
                if (*curr_head < deco->slot_size) break; // Stop adding. Deco can no longer fit.

                curr_decos.emplace_back(deco);
                ++curr_head;

                if (curr_head == deco_slots.end()) {
                    complete_combos.emplace_back(std::move(curr_decos));
                    break;
                } else {
                    new_incomplete_combos.emplace_back(std::move(curr_decos),
                                                       std::move(curr_head) );
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


static std::vector<ArmourPieceCombo> generate_slot_combos(const std::vector<const ArmourPiece*>& pieces,
                                                          const std::array<std::vector<const Decoration*>,
                                                                           k_MAX_DECO_SIZE>& sorted_decos,
                                                          const SkillSpec& skill_spec,
                                                          const std::unordered_set<const SetBonus*>& set_bonus_subset,
                                                          const std::string& debug_msg) {
    // We will assume it's sorted.
    // TODO: Add a runtime assert.

    unsigned long long stat_pre = 0;

    SkillsSeenSet<ArmourPieceCombo> seen_set;

    for (const ArmourPiece* piece : pieces) {
        std::vector<std::vector<const Decoration*>> deco_combos = generate_deco_combos(piece->deco_slots,
                                                                                       sorted_decos,
                                                                                       skill_spec);
        SkillMap armour_skills;
        armour_skills.add_skills_filtered(*piece, skill_spec);

        stat_pre += deco_combos.size(); // TODO: How do I know this won't overflow?

        for (std::vector<const Decoration*>& decos : deco_combos) {
            SkillMap skills = armour_skills;
            skills.add_skills_filtered(decos, skill_spec);

            SkillsAndSetBonuses h = {skills, {}};
            if (Utils::set_has_key(set_bonus_subset, piece->set_bonus)) {
                h.set_bonuses[piece->set_bonus] = 1;
            }

            assert(h.skills == skills);
            seen_set.add(std::move(h), {piece, std::move(decos), std::move(skills)});
        }
    }

    std::vector<ArmourPieceCombo> ret = seen_set.get_data_as_vector();

    Utils::log_stat_reduction(debug_msg, stat_pre, ret.size());

    return ret;
}


//static std::vector<ArmourSetCombo> merge_in_armour_list(SkillsSeenSet<ArmourSetCombo>& armour_combos,
//                                                        const std::vector<ArmourPieceCombo>& pieces,
//                                                        const SkillSpec& skill_spec) {
//
//    std::vector<ArmourSetCombo> prev_combos = armour_combos.get_data_as_vector();
//    
//    // TODO
//
//    return new_combos;
//}


static void do_search(const Database& db, const SearchParameters& params) {

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

    std::vector<const Weapon*> weapons = prepare_weapons(db, params);
    assert(weapons.size());

    std::array<std::vector<const Decoration*>, k_MAX_DECO_SIZE> grouped_sorted_decos = prepare_decos(db, params.skill_spec);
    assert(grouped_sorted_decos.size() == 4);
    assert(grouped_sorted_decos[0].size());
    assert(grouped_sorted_decos[1].size());
    assert(grouped_sorted_decos[2].size());
    assert(grouped_sorted_decos[3].size());

    std::vector<const Charm*> charms = prepare_charms(db, params.skill_spec);
    assert(charms.size());

    std::map<ArmourSlot, std::vector<const ArmourPiece*>> armour = get_pruned_armour(db, params);

    assert(armour.size() == 5);
    assert(armour.at(ArmourSlot::head).size());
    assert(armour.at(ArmourSlot::chest).size());
    assert(armour.at(ArmourSlot::arms).size());
    assert(armour.at(ArmourSlot::waist).size());
    assert(armour.at(ArmourSlot::legs).size());

    std::vector<ArmourPieceCombo> head_combos = generate_slot_combos(armour.at(ArmourSlot::head),
                                                                     grouped_sorted_decos,
                                                                     params.skill_spec,
                                                                     set_bonus_subset,
                                                                     "Generated head+deco combinations:  ");
    std::vector<ArmourPieceCombo> chest_combos = generate_slot_combos(armour.at(ArmourSlot::chest),
                                                                      grouped_sorted_decos,
                                                                      params.skill_spec,
                                                                      set_bonus_subset,
                                                                      "Generated chest+deco combinations: ");
    std::vector<ArmourPieceCombo> arms_combos = generate_slot_combos(armour.at(ArmourSlot::arms),
                                                                     grouped_sorted_decos,
                                                                     params.skill_spec,
                                                                     set_bonus_subset,
                                                                     "Generated arms+deco combinations:  ");
    std::vector<ArmourPieceCombo> waist_combos = generate_slot_combos(armour.at(ArmourSlot::waist),
                                                                      grouped_sorted_decos,
                                                                      params.skill_spec,
                                                                      set_bonus_subset,
                                                                     "Generated waist+deco combinations: ");
    std::vector<ArmourPieceCombo> legs_combos = generate_slot_combos(armour.at(ArmourSlot::legs),
                                                                     grouped_sorted_decos,
                                                                     params.skill_spec,
                                                                     set_bonus_subset,
                                                                     "Generated legs+deco combinations:  ");

    // We build the initial build list.
    
    std::vector<ArmourSetCombo> combos;

    for (const Charm* charm : charms) {
        combos.emplace_back(charm);
    }

    SkillsSeenSet<ArmourSetCombo> armour_combos;

    // And now, we merge in our slot combinations!
}


void search_cmd(const std::string& search_parameters_path) {

    const Database db = Database::get_db();
    const SearchParameters params = read_file(db, search_parameters_path);

    do_search(db, params);
}


} // namespace

