/*
 * File: mhwi_build_search.cpp
 * Author: <contact@simshadows.com>
 */

#include "utils.h"

#include <assert.h>

#include <iostream>
#include <cmath>


constexpr double k_GREATSWORD_BLOAT       = 4.8;
constexpr double k_LONGSWORD_BLOAT        = 3.3;
constexpr double k_SWORD_AND_SHIELD_BLOAT = 1.4;
constexpr double k_DUAL_BLADES_BLOAT      = 1.4;
constexpr double k_HAMMER_BLOAT           = 5.2;
constexpr double k_HUNTING_HORN_BLOAT     = 4.2;
constexpr double k_LANCE_BLOAT            = 2.3;
constexpr double k_GUNLANCE_BLOAT         = 2.3;
constexpr double k_SWITCHAXE_BLOAT        = 3.5;
constexpr double k_CHARGE_BLADE_BLOAT     = 3.6;
constexpr double k_INSECT_GLAIVE_BLOAT    = 4.1;
constexpr double k_BOW_BLOAT              = 1.2;
constexpr double k_HEAVY_BOWGUN_BLOAT     = 1.5;
constexpr double k_LIGHT_BOWGUN_BLOAT     = 1.3;

constexpr double k_RAW_BLUNDER_MULTIPLIER = 0.75;
constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB0 = 1.25; // Critical Boost 0
constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB1 = 1.30; // Critical Boost 1
constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB2 = 1.35; // Critical Boost 2
constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB3 = 1.40; // Critical Boost 3

constexpr double k_RAW_SHARPNESS_MODIFIER_RED    = 0.50;
constexpr double k_RAW_SHARPNESS_MODIFIER_ORANGE = 0.75;
constexpr double k_RAW_SHARPNESS_MODIFIER_YELLOW = 1.00;
constexpr double k_RAW_SHARPNESS_MODIFIER_GREEN  = 1.05;
constexpr double k_RAW_SHARPNESS_MODIFIER_BLUE   = 1.20;
constexpr double k_RAW_SHARPNESS_MODIFIER_WHITE  = 1.32;
constexpr double k_RAW_SHARPNESS_MODIFIER_PURPLE = 1.39;

constexpr double k_NON_ELEMENTAL_BOOST_MULTIPLIER = 1.05;

constexpr unsigned int k_POWERCHARM_RAW = 6;
constexpr unsigned int k_POWERTALON_RAW = 9;


/*
 * Weapons
 */


enum class WeaponType {
    greatsword,
    longsword,
    sword_and_shield,
    dual_blades,
    hammer,
    hunting_horn,
    lance,
    gunlance,
    switchaxe,
    charge_blade,
    insect_glaive,
    bow,
    heavy_bowgun,
    light_bowgun,
};


double weapontype_to_bloat_value(WeaponType wt) {
    switch (wt) {
        case WeaponType::greatsword:
            return k_GREATSWORD_BLOAT;
        case WeaponType::longsword:
            return k_LONGSWORD_BLOAT;
        case WeaponType::sword_and_shield:
            return k_SWORD_AND_SHIELD_BLOAT;
        case WeaponType::dual_blades:
            return k_DUAL_BLADES_BLOAT;
        case WeaponType::hammer:
            return k_HAMMER_BLOAT;
        case WeaponType::hunting_horn:
            return k_HUNTING_HORN_BLOAT;
        case WeaponType::lance:
            return k_LANCE_BLOAT;
        case WeaponType::gunlance:
            return k_GUNLANCE_BLOAT;
        case WeaponType::switchaxe:
            return k_SWITCHAXE_BLOAT;
        case WeaponType::charge_blade:
            return k_CHARGE_BLADE_BLOAT;
        case WeaponType::insect_glaive:
            return k_INSECT_GLAIVE_BLOAT;
        case WeaponType::bow:
            return k_BOW_BLOAT;
        case WeaponType::heavy_bowgun:
            return k_HEAVY_BOWGUN_BLOAT;
        case WeaponType::light_bowgun:
            return k_LIGHT_BOWGUN_BLOAT;
        default:
            throw "invalid weapon type";
    }
}


/*
 * Everything Else
 */


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

    double weapon_bloat = weapontype_to_bloat_value(WeaponType::greatsword);

    unsigned int weapon_bloated_raw = 1296;
    unsigned int weapon_raw = weapon_bloated_raw / weapon_bloat;
    unsigned int weapon_aff = 15;

    double raw_multiplier = k_NON_ELEMENTAL_BOOST_MULTIPLIER;
    unsigned int added_raw = k_POWERCHARM_RAW + k_POWERTALON_RAW;
    unsigned int added_aff = 10;

    double raw_sharpness_modifier = k_RAW_SHARPNESS_MODIFIER_WHITE;
    double raw_crit_dmg_multiplier = k_RAW_CRIT_DMG_MULTIPLIER_CB0;

    double efr = calculate_efr(weapon_raw,
                               weapon_aff,
                               raw_multiplier,
                               added_raw,
                               added_aff,
                               raw_sharpness_modifier,
                               raw_crit_dmg_multiplier);

    std::clog << efr << std::endl;

    assert(Utils::round_2decpl(efr) == 419.35); // Quick test!

    return 0;
}

