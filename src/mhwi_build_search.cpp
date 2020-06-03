/*
 * File: mhwi_build_search.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <cstring>
#include <iostream>
#include <cmath>
#include <chrono>
#include <vector>

#include "mhwi_build_search.h"
#include "core/core.h"
#include "database/database.h"
#include "database/database_skills.h"
#include "support/support.h"
#include "utils/utils.h"
#include "utils/logging.h"


namespace MHWIBuildSearch
{


void no_args_cmd() {
    const Database db = Database::get_db();

    std::unordered_map<const Skill*, unsigned int> min_levels = {
        {&SkillsDatabase::g_skill_weakness_exploit, 0},
        {&SkillsDatabase::g_skill_agitator, 0},
        {&SkillsDatabase::g_skill_fortify, 0},
        {&SkillsDatabase::g_skill_frostcraft, 0},
        {&SkillsDatabase::g_skill_heroics, 0},
        {&SkillsDatabase::g_skill_airborne, 0},
    };
    std::unordered_map<const Skill*, unsigned int> forced_states = {
        {&SkillsDatabase::g_skill_fortify, 1},
    };
    SkillSpec skill_spec(std::move(min_levels), std::move(forced_states), {});
    std::clog << std::endl << skill_spec.get_humanreadable() << std::endl << std::endl;

    /*
     * Using values for Royal Venus Blade with only one affinity augment and Elementless Jewel 2.
     */

    WeaponInstance weapon(db.weapons.at("SAFI_SHATTERSPLITTER"));
    weapon.augments->set_augment(WeaponAugment::augment_lvl, 3);
    weapon.augments->set_augment(WeaponAugment::attack_increase, 1);
    weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_sharpness_6);
    weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_sharpness_5);
    weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_sharpness_5);

    ArmourEquips armour;
    armour.add(db.armour.at("Raging Brachy",
                            Tier::master_rank,
                            ArmourVariant::master_rank_beta_plus,
                            ArmourSlot::head));
    armour.add(db.armour.at("Teostra",
                            Tier::master_rank,
                            ArmourVariant::master_rank_beta_plus,
                            ArmourSlot::arms));
    armour.add(db.charms.at("CHALLENGER_CHARM"));

    DecoEquips decos;
    
    std::clog << weapon.get_humanreadable() << std::endl << std::endl;
    std::clog << armour.get_humanreadable() << std::endl << std::endl;
    std::clog << armour.get_skills_without_set_bonuses().get_humanreadable() << std::endl << std::endl;

    EffectiveDamageValues edv = calculate_edv_from_gear_lookup(weapon, armour, decos, skill_spec);
    std::clog << edv.get_humanreadable() << "\n\n===================\n\n";

    /*
     * For testing purposes, we'll also do a by-skill lookup.
     */

    weapon = WeaponInstance(db.weapons.at("ROYAL_VENUS_BLADE"));

    SkillMap skills;
    skills.set(&SkillsDatabase::g_skill_agitator, 1);
    skills.set(&SkillsDatabase::g_skill_critical_boost, 3);
    skills.set(&SkillsDatabase::g_skill_non_elemental_boost, 1);
    skills.set(&SkillsDatabase::g_skill_fortify, 1);
    skills.set(&SkillsDatabase::g_skill_frostcraft, 1);
    skills.set(&SkillsDatabase::g_skill_heroics, 5);
    skills.set(&SkillsDatabase::g_skill_airborne, 1);

    WeaponContribution wc = weapon.calculate_contribution();
    std::clog << std::endl << skill_spec.get_humanreadable() + "\n\n";
    std::clog << skills.get_humanreadable() + "\n\n";
    edv = calculate_edv_from_skills_lookup(weapon.weapon->weapon_class, wc, skills, skill_spec);
    std::clog << edv.get_humanreadable() << std::endl;
    //assert(Utils::round_2decpl(efr) == 437.85); // Quick test!
}


} // namespace


int main(int argc, char** argv) {
    assert(fprintf(stderr, "[WARNING] Assertions are enabled. "
                   "Program performance may be impacted.\n\n") > 0);

    auto start_t = std::chrono::steady_clock::now();

    if ((argc == 3) && (std::strcmp(argv[1], "search") == 0)) {
        MHWIBuildSearch::search_cmd(std::string(argv[2]));
    } else if (argc == 1) {
        MHWIBuildSearch::no_args_cmd();
    } else {
        std::cerr << "Invalid command arguments." << std::endl;
        return 1;
    }

    std::clog << std::endl;
    Utils::log_stat_duration("Total execution time: ", start_t);
    Utils::log_stat();

    return 0;
}

