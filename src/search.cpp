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
#include "utils/utils.h"


namespace MHWIBuildSearch
{


/*
 * extract_all_skills implementations
 */


//static std::unordered_set<const Skill*> extract_all_skills(const std::vector<const SetBonus*>& v) {
//    std::unordered_set<const Skill*> ret;
//    for (const SetBonus* set_bonus : v) {
//        for (const auto& e : set_bonus->stages) {
//            assert(e.first > 0);
//            ret.emplace(e.second);
//        }
//    }
//    return ret;
//}
//
//static std::unordered_set<const Skill*> extract_all_skills(const std::vector<const Decoration*>& v) {
//    std::unordered_set<const Skill*> ret;
//    for (const Decoration* deco : v) {
//        for (const auto& e : deco->skills) {
//            assert(e.second > 0);
//            ret.emplace(e.first);
//        }
//    }
//    return ret;
//}
//
//static std::unordered_set<const Skill*> extract_all_skills(const std::vector<const Weapon*>& v) {
//    std::unordered_set<const Skill*> ret;
//    for (const Weapon* weapon : v) {
//        if (weapon->skill) {
//            ret.emplace(weapon->skill);
//        }
//    }
//    return ret;
//}
//
//static std::unordered_set<const Skill*> extract_all_skills(const std::vector<const ArmourPiece*>& v) {
//    std::unordered_set<const Skill*> ret;
//    for (const ArmourPiece* piece : v) {
//        for (const auto& e : piece->skills) {
//            ret.emplace(e.first);
//        }
//    }
//    return ret;
//}
//
//static std::unordered_set<const Skill*> extract_all_skills(const std::vector<const Charm*>& v) {
//    std::unordered_set<const Skill*> ret;
//    for (const Charm* charm : v) {
//        for (const Skill* skill : charm->skills) {
//            ret.emplace(skill);
//        }
//    }
//    return ret;
//}


/*
 * Search implementation
 */


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


static std::vector<const Charm*> get_pruned_charms(const Database& db, const SkillSpec& skill_spec) {
    std::vector<const Charm*> ret = db.charms.get_all();

    const std::size_t stat_pre = ret.size();

    const auto pred = [&](const Charm* x){for (const Skill* s : x->skills)
                                              if (skill_spec.is_in_subset(s))
                                                  return false;
                                          return true; };
    ret.erase(std::remove_if(ret.begin(), ret.end(), pred), ret.end());

    const std::size_t stat_post = ret.size();

    Utils::log_stat_reduction("Charms pruning: ", stat_pre, stat_post);
    Utils::log_stat();

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
    Utils::log_stat();

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


//static std::vector<std::vector<const Decoration*>> get_deco_combos(const std::vector<unsigned int>& sorted_slots,
//                                                                   const std::vector<const Decoration*>& sorted_decos) {
//    assert(std::is_sorted(sorted_slots.begin(), sorted_slots.end(), std::greater<unsigned int>()));
//    (void)sorted_decos;
//    std::vector<std::vector<const Decoration*>> ret;
//    return ret;
//}


//static std::vector<ArmourSetCombo> merge_in_armour_list(const std::vector<ArmourSetCombo>& prev_combos,
//                                                         const std::vector<ArmourPieceCombo>& pieces,
//                                                         const SkillSpec& skill_spec) {
//    std::vector<ArmourSetCombo> new_combos;
//    (void)prev_combos;
//    (void)pieces;
//    (void)skill_spec;
//
//    return new_combos;
//}


static std::vector<std::vector<const Decoration*>> generate_deco_combos(const std::vector<unsigned int>& deco_slots,
                                                                        const std::array<std::vector<const Decoration*>,
                                                                                         k_MAX_DECO_SIZE>& sorted_decos,
                                                                        const SkillSpec skill_spec) {
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
    std::vector<const Decoration*> deco_sublist = sorted_decos[sublist_index];

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
                                                          const SkillSpec& skill_spec) {
    // We will assume it's sorted.
    // TODO: Add a runtime assert.

    std::vector<ArmourPieceCombo> ret;

    for (const ArmourPiece* piece : pieces) {
        std::vector<std::vector<const Decoration*>> deco_combos = generate_deco_combos(piece->deco_slots,
                                                                                       sorted_decos,
                                                                                       skill_spec);
        for (std::vector<const Decoration*>& decos : deco_combos) {
            SkillMap skills (*piece);
            // TODO: Use DecoEquips here?
            for (const Decoration * const deco : decos) {
                for (const auto& e : deco->skills) {
                    skills.increment_lvl(e.first, e.second);
                }
            }

            ret.push_back({piece, std::move(decos), std::move(skills)});
        }
    }

    return ret;
}


static void do_search(const Database& db, const SearchParameters& params) {

    std::clog << params.skill_spec.get_humanreadable() << std::endl << std::endl;

    // Determine what skills absolutely have to be served by set bonus.

    std::unordered_set<const SetBonus*> set_bonus_subset;

    {
        const std::vector<const SetBonus*> all_set_bonuses = db.skills.get_all_set_bonuses();

        //const std::unordered_set<const Skill*> sb_skills     = extract_all_skills(all_set_bonuses);

        //const std::unordered_set<const Skill*> deco_skills   = extract_all_skills(db.decos.get_all());
        //const std::unordered_set<const Skill*> weapon_skills = extract_all_skills(db.weapons.get_all());
        //const std::unordered_set<const Skill*> armour_skills = extract_all_skills(db.armour.get_all_pieces());
        //const std::unordered_set<const Skill*> charm_skills  = extract_all_skills(db.charms.get_all());

        //std::unordered_set<const Skill*> non_sb_skills = deco_skills;     // Set union
        //non_sb_skills.insert(weapon_skills.begin(), weapon_skills.end()); // Set union
        //non_sb_skills.insert(armour_skills.begin(), armour_skills.end()); // Set union
        //non_sb_skills.insert(charm_skills.begin(),  charm_skills.end());  // Set union

        //std::unordered_set<const Skill*> only_possible_with_sb = Utils::set_diff(sb_skills, non_sb_skills);

        //Utils::log_stat("Total skills serveable by decorations: ", deco_skills.size()  );
        //Utils::log_stat("Total skills serveable by weapons:     ", weapon_skills.size());
        //Utils::log_stat("Total skills serveable by armour:      ", armour_skills.size());
        //Utils::log_stat("Total skills serveable by charms:      ", charm_skills.size() );
        //Utils::log_stat();
        //Utils::log_stat("Total skills serveable by set bonuses:      ", sb_skills.size()    );
        //Utils::log_stat("Total skills serveable without set bonuses: ", non_sb_skills.size());
        //Utils::log_stat();
        //Utils::log_stat("Total skills that can only be served by set bonuses: ", only_possible_with_sb.size());

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

    std::vector<const Weapon*> weapons = db.weapons.get_all_of_weaponclass(params.weapon_class);
    std::map<ArmourSlot, std::vector<const ArmourPiece*>> armour = db.armour.get_all_pieces_by_slot();

    assert(armour.size() == 5);
    assert(armour.at(ArmourSlot::head).size());
    assert(armour.at(ArmourSlot::chest).size());
    assert(armour.at(ArmourSlot::arms).size());
    assert(armour.at(ArmourSlot::waist).size());
    assert(armour.at(ArmourSlot::legs).size());
    assert(weapons.size());
    Utils::log_stat("Total weapons: ", weapons.size());
    Utils::log_stat();
    Utils::log_stat("Total head pieces:  ", armour.at(ArmourSlot::head).size());
    Utils::log_stat("Total chest pieces: ", armour.at(ArmourSlot::chest).size());
    Utils::log_stat("Total arm pieces:   ", armour.at(ArmourSlot::arms).size());
    Utils::log_stat("Total waist pieces: ", armour.at(ArmourSlot::waist).size());
    Utils::log_stat("Total leg pieces:   ", armour.at(ArmourSlot::legs).size());
    Utils::log_stat();

    std::array<std::vector<const Decoration*>, k_MAX_DECO_SIZE> grouped_sorted_decos = prepare_decos(db, params.skill_spec);

    std::vector<const Charm*> charms = get_pruned_charms(db, params.skill_spec);
    assert(charms.size());

    // We build the initial build list.
    
    std::vector<ArmourSetCombo> combos;

    for (const Charm* charm : charms) {
        combos.emplace_back(charm);
    }

    // And now, we start adding stuff!

    std::vector<ArmourPieceCombo> head_combos = generate_slot_combos(armour.at(ArmourSlot::head),
                                                                     grouped_sorted_decos,
                                                                     params.skill_spec);
    std::vector<ArmourPieceCombo> chest_combos = generate_slot_combos(armour.at(ArmourSlot::chest),
                                                                      grouped_sorted_decos,
                                                                      params.skill_spec);
    std::vector<ArmourPieceCombo> arms_combos = generate_slot_combos(armour.at(ArmourSlot::arms),
                                                                     grouped_sorted_decos,
                                                                     params.skill_spec);
    std::vector<ArmourPieceCombo> waist_combos = generate_slot_combos(armour.at(ArmourSlot::waist),
                                                                      grouped_sorted_decos,
                                                                      params.skill_spec);
    std::vector<ArmourPieceCombo> legs_combos = generate_slot_combos(armour.at(ArmourSlot::legs),
                                                                     grouped_sorted_decos,
                                                                     params.skill_spec);
    Utils::log_stat("Generated head combinations:  ", head_combos.size());
    Utils::log_stat("Generated chest combinations: ", chest_combos.size());
    Utils::log_stat("Generated arms combinations:  ", arms_combos.size());
    Utils::log_stat("Generated waist combinations: ", waist_combos.size());
    Utils::log_stat("Generated legs combinations:  ", legs_combos.size());

    // TODO: Continue!
}


void search_cmd(const std::string& search_parameters_path) {

    const Database db = Database::get_db();
    const SearchParameters params = read_file(db, search_parameters_path);

    do_search(db, params);
}


} // namespace

