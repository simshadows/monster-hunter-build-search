/*
 * File: mhwi_build_search.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include <iostream>
#include <cmath>

#include "core/core.h"
#include "database/database.h"
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
                            double       raw_crit_dmg_multiplier,
                            double       raw_sharpness_modifier) {

    assert(weapon_raw > 0);
    assert(neb_multiplier > 0.0);
    assert(raw_crit_dmg_multiplier > 0.0);
    assert(raw_sharpness_modifier > 0.0);

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
    
    double efr = true_raw * raw_crit_modifier * raw_sharpness_modifier;
    return efr;
}


double calculate_efr_from_skills_lookup(const Database::Database&     db,
                                        const Database::Weapon&       weapon,
                                        const WeaponAugmentsInstance& weapon_augs,
                                        const SkillMap&               skills,
                                        const SkillSpec&              skill_spec) {

    SkillContribution sc(db, skills, skill_spec, weapon, weapon.maximum_sharpness);
    WeaponAugmentsContribution wac = weapon_augs.calculate_contribution();
    return calculate_efr(weapon.true_raw,
                         weapon.affinity,
                         sc.neb_multiplier,
                         sc.added_raw + wac.added_raw + k_POWERCHARM_RAW + k_POWERTALON_RAW,
                         sc.added_aff + wac.added_aff,
                         sc.raw_crit_dmg_multiplier,
                         sc.raw_sharpness_modifier);
}


double calculate_efr_from_gear_lookup(const Database::Database&     db,
                                      const Database::Weapon&       weapon,
                                      const WeaponAugmentsInstance& weapon_augs,
                                      const ArmourEquips&           armour,
                                      const SkillSpec&              skill_spec) {
    SkillMap skills = armour.get_skills_without_set_bonuses();
    return calculate_efr_from_skills_lookup(db, weapon, weapon_augs, skills, skill_spec);
}


void run() {
    const Database::Database db = Database::Database::get_db();

    std::unordered_map<const Database::Skill*, unsigned int> min_levels = {
        {db.weakness_exploit_ptr, 0},
        {db.agitator_ptr, 0},
    };
    std::unordered_map<const Database::Skill*, unsigned int> forced_states;
    SkillSpec skill_spec(std::move(min_levels), std::move(forced_states));
    std::clog << std::endl << skill_spec.get_humanreadable() << std::endl << std::endl;

    /*
     * Using values for Royal Venus Blade with only one affinity augment and Elementless Jewel 2.
     */

    const Database::Weapon* weapon = db.weapons.at("ROYAL_VENUS_BLADE");
    std::unique_ptr<WeaponAugmentsInstance> wa = WeaponAugmentsInstance::get_instance(*weapon);

    ArmourEquips armour;
    armour.add(db.armour.at("Raging Brachy",
                            Database::Tier::master_rank,
                            Database::ArmourVariant::master_rank_beta_plus,
                            Database::ArmourSlot::head));
    armour.add(db.armour.at("Teostra",
                            Database::Tier::master_rank,
                            Database::ArmourVariant::master_rank_beta_plus,
                            Database::ArmourSlot::arms));
    armour.add(db.charms.at("CHALLENGER_CHARM"));
    
    std::clog << wa->get_humanreadable() << std::endl << std::endl;
    std::clog << armour.get_humanreadable() << std::endl << std::endl;
    std::clog << armour.get_skills_without_set_bonuses().get_humanreadable() << std::endl << std::endl;

    double efr = calculate_efr_from_gear_lookup(db, *weapon, *wa, armour, skill_spec);
    std::clog << efr << std::endl;

    /*
     * For testing purposes, we'll also do a by-skill lookup.
     */

    weapon = db.weapons.at("ROYAL_VENUS_BLADE");
    wa = WeaponAugmentsInstance::get_instance(*weapon);

    SkillMap skills;
    skills.set_lvl(db.critical_boost_ptr, 3);
    skills.set_lvl(db.non_elemental_boost_ptr, 1);
    //skills.set_lvl(db.true_element_acceleration_ptr, 1);

    efr = calculate_efr_from_skills_lookup(db, *weapon, *wa, skills, skill_spec);
    std::clog << efr << std::endl;
    //assert(Utils::round_2decpl(efr) == 437.85); // Quick test!
}


} // namespace


int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    MHWIBuildSearch::run();

    return 0;
}

