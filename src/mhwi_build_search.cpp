/*
 * File: mhwi_build_search.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include <iostream>
#include <cmath>

#include "containers.h"
#include "database/database.h"
#include "skill_contributions.h"
#include "utils.h"


namespace MHWIBuildSearch
{


constexpr double k_RAW_BLUNDER_MULTIPLIER = 0.75;

constexpr unsigned int k_POWERCHARM_RAW = 6;
constexpr unsigned int k_POWERTALON_RAW = 9;


static double calculate_efr(unsigned int weapon_raw, // True raw, not bloated raw.
                            int          weapon_aff,
                            double       neb_multiplier,
                            unsigned int added_raw,
                            int          added_aff,
                            double       raw_sharpness_modifier,
                            double       raw_crit_dmg_multiplier) {

    assert(weapon_raw > 0);
    assert(neb_multiplier > 0.0);
    assert(raw_sharpness_modifier > 0.0);
    assert(raw_crit_dmg_multiplier > 0.0);

    //double raw_crit_chance = std::clamp(((double) (weapon_aff + added_aff)) / 100, -1.0, 1.0); // C++17
    double raw_crit_chance = ((double) (weapon_aff + added_aff)) / 100;
    double raw_crit_modifier;
    if (raw_crit_chance < 0) {
        double raw_blunder_chance = (raw_crit_chance < -1.0) ? -1.0 : -raw_crit_chance;
        raw_crit_modifier = (k_RAW_BLUNDER_MULTIPLIER * raw_blunder_chance) + (1 - raw_blunder_chance);
    } else {
        raw_crit_chance = (raw_crit_chance > 1.0) ? 1.0 : raw_crit_chance;
        raw_crit_modifier = (raw_crit_dmg_multiplier * raw_crit_chance) + (1 - raw_crit_chance);
    }

    double weapon_multiplied_raw = weapon_raw * neb_multiplier;
    unsigned int true_raw = std::round(weapon_multiplied_raw) + added_raw;
    
    double efr = true_raw * raw_sharpness_modifier * raw_crit_modifier;
    return efr;
}


double calculate_efr_from_skills_lookup(const Database::Database& db,
                                        const Database::Weapon& weapon,
                                        const SkillMap& skills) {

    double neb_multiplier = calculate_non_elemental_boost_multiplier(db, skills, weapon);
    double raw_crit_dmg_multiplier = calculate_raw_crit_dmg_multiplier(db, skills);
    double raw_sharpness_modifier = calculate_raw_sharpness_modifier(db, skills, weapon.maximum_sharpness);

    unsigned int added_raw = k_POWERCHARM_RAW + k_POWERTALON_RAW;
    unsigned int added_aff = 0;

    return calculate_efr(weapon.true_raw,
                         weapon.affinity,
                         neb_multiplier,
                         added_raw,
                         added_aff,
                         raw_sharpness_modifier,
                         raw_crit_dmg_multiplier);
}


double calculate_efr_from_gear_lookup(const Database::Database& db,
                                      const Database::Weapon& weapon,
                                      const ArmourEquips& armour) {
    SkillMap skills = armour.get_skills_without_set_bonuses();
    return calculate_efr_from_skills_lookup(db, weapon, skills);
}


} // namespace


int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    const Database::Database db = Database::Database::get_db();

    /*
     * Using values for Royal Venus Blade with only one affinity augment and Elementless Jewel 2.
     */

    const Database::Weapon* weapon = db.weapons.at("ROYAL_VENUS_BLADE");
    
    MHWIBuildSearch::ArmourEquips armour;
    armour.add(db.armour.at("Raging Brachy",
                            Database::Tier::master_rank,
                            Database::ArmourVariant::master_rank_beta_plus,
                            Database::ArmourSlot::head));
    armour.add(db.armour.at("Teostra",
                            Database::Tier::master_rank,
                            Database::ArmourVariant::master_rank_beta_plus,
                            Database::ArmourSlot::arms));
    
    std::clog << armour.get_humanreadable() << std::endl << std::endl;
    std::clog << armour.get_skills_without_set_bonuses().get_humanreadable() << std::endl << std::endl;

    double efr = MHWIBuildSearch::calculate_efr_from_gear_lookup(db, *weapon, armour);
    std::clog << efr << std::endl;
    assert(Utils::round_2decpl(efr) == 390.31); // Quick test!

    /*
     * For testing purposes, we'll also do a by-skill lookup.
     */

    weapon = db.weapons.at("GREAT_DEMON_ROD");

    MHWIBuildSearch::SkillMap skills;
    skills.set_lvl(db.non_elemental_boost_ptr, 1);

    efr = MHWIBuildSearch::calculate_efr_from_skills_lookup(db, *weapon, skills);
    std::clog << efr << std::endl;
    assert(Utils::round_2decpl(efr) == 375.38); // Quick test!

    return 0;
}

