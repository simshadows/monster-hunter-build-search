/*
 * File: mhwi_build_search.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include <iostream>
#include <cmath>

#include "database/database.h"
#include "utils.h"


constexpr double k_RAW_BLUNDER_MULTIPLIER = 0.75;
constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB0 = 1.25; // Critical Boost 0
//constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB1 = 1.30; // Critical Boost 1
//constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB2 = 1.35; // Critical Boost 2
//constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB3 = 1.40; // Critical Boost 3

constexpr double k_NON_ELEMENTAL_BOOST_MULTIPLIER = 1.05;

constexpr unsigned int k_POWERCHARM_RAW = 6;
constexpr unsigned int k_POWERTALON_RAW = 9;


static double calculate_efr(unsigned int weapon_raw, // True raw, not bloated raw.
                            int          weapon_aff,
                            double       raw_multiplier,
                            unsigned int added_raw,
                            int          added_aff,
                            double       raw_sharpness_modifier,
                            double       raw_crit_dmg_multiplier) {

    assert(weapon_raw > 0);
    assert(raw_multiplier > 0.0);
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

    double weapon_multiplied_raw = weapon_raw * raw_multiplier;
    unsigned int true_raw = std::round(weapon_multiplied_raw) + added_raw;
    
    double efr = true_raw * raw_sharpness_modifier * raw_crit_modifier;
    return efr;
}


double calculate_efr_from_skills_lookup(const Database::Weapon& weapon) {
    double raw_multiplier = k_NON_ELEMENTAL_BOOST_MULTIPLIER; // Arbitrary for testing
    unsigned int added_raw = k_POWERCHARM_RAW + k_POWERTALON_RAW;
    unsigned int added_aff = 10; // Arbitrary for testing
    unsigned int handicraft_lvl = 0;
    double raw_crit_dmg_multiplier = k_RAW_CRIT_DMG_MULTIPLIER_CB0;

    return calculate_efr(weapon.true_raw,
                         weapon.affinity,
                         raw_multiplier,
                         added_raw,
                         added_aff,
                         weapon.maximum_sharpness.get_raw_sharpness_modifier(handicraft_lvl),
                         raw_crit_dmg_multiplier);
}


int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    const Database::Database db = Database::Database::get_db();

    /*
     * Using values for Royal Venus Blade with only one affinity augment and Elementless Jewel 2.
     */

    const Database::Weapon* weapon = db.weapons.at("WYVERN_IMPACT_SILVER");

    double efr = calculate_efr_from_skills_lookup(*weapon);

    std::clog << efr << std::endl;

    // TODO: Learn why this is rounding 410.415 DOWN to 410.41. This is so weird!
    assert(Utils::round_2decpl(efr) == 410.41); // Quick test!

    return 0;
}

