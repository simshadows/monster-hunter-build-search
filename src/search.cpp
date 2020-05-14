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


std::vector<const Charm*> get_pruned_charms(const Database& db, const SkillSpec& skill_spec) {
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


std::vector<const Decoration*> get_pruned_decos(const Database& db, const SkillSpec& skill_spec) {
    std::vector<const Decoration*> ret = db.decos.get_all();

    const std::size_t stat_pre = ret.size();

    const auto pred = [&](const Decoration* x){for (const auto& e : x->skills)
                                                   if (skill_spec.is_in_subset(e.first)) {
                                                       assert(e.second > 0);
                                                       return false;
                                                   }
                                               return true; };
    ret.erase(std::remove_if(ret.begin(), ret.end(), pred), ret.end());

    const std::size_t stat_post = ret.size();

    Utils::log_stat_reduction("Decorations pruning: ", stat_pre, stat_post);
    Utils::log_stat();

    return ret;
}


static void do_search(const Database& db, const SearchParameters& params) {

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

    //std::vector<const Charm*> charms = db.charms.get_all();
    std::vector<const Charm*> charms = get_pruned_charms(db, params.skill_spec);
    std::vector<const Decoration*> decos = get_pruned_decos(db, params.skill_spec);

    assert(charms.size());
    assert(decos.size());

    // TODO: Continue!
}


void search_cmd(const std::string& search_parameters_path) {

    const Database db = Database::get_db();
    const SearchParameters params = read_file(db, search_parameters_path);

    do_search(db, params);
}


} // namespace

