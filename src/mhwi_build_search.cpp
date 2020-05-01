/*
 * File: mhwi_build_search.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include <iostream>
#include <cmath>

#include "database_weapons.h"
#include "utils.h"


constexpr double k_RAW_BLUNDER_MULTIPLIER = 0.75;
constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB0 = 1.25; // Critical Boost 0
constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB1 = 1.30; // Critical Boost 1
constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB2 = 1.35; // Critical Boost 2
constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB3 = 1.40; // Critical Boost 3

constexpr double k_NON_ELEMENTAL_BOOST_MULTIPLIER = 1.05;

constexpr unsigned int k_POWERCHARM_RAW = 6;
constexpr unsigned int k_POWERTALON_RAW = 9;


double calculate_efr(unsigned int weapon_raw, // True raw, not bloated raw.
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


int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    /*
     * Using values for Royal Venus Blade with only one affinity augment and Elementless Jewel 2.
     */

    double weapon_bloat = Weapons::weapontype_to_bloat_value(Weapons::WeaponType::greatsword);

    unsigned int weapon_bloated_raw = 1296;
    Weapons::SharpnessGauge original_sharpness(150, 30, 30, 60, 50, 30, 50);

    unsigned int weapon_raw = weapon_bloated_raw / weapon_bloat;
    unsigned int weapon_aff = 15;

    double raw_multiplier = k_NON_ELEMENTAL_BOOST_MULTIPLIER;
    unsigned int added_raw = k_POWERCHARM_RAW + k_POWERTALON_RAW;
    unsigned int added_aff = 10;

    Weapons::SharpnessGauge new_sharpness = original_sharpness.apply_handicraft(0);
    double raw_sharpness_modifier = new_sharpness.get_raw_sharpness_modifier();
    double raw_crit_dmg_multiplier = k_RAW_CRIT_DMG_MULTIPLIER_CB0;

    double efr = calculate_efr(weapon_raw,
                               weapon_aff,
                               raw_multiplier,
                               added_raw,
                               added_aff,
                               raw_sharpness_modifier,
                               raw_crit_dmg_multiplier);

    std::clog << new_sharpness.get_humanreadable() << std::endl;
    std::clog << efr << std::endl;

    assert(Utils::round_2decpl(efr) == 419.35); // Quick test!

    return 0;
}

