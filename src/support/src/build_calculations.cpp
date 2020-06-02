/*
 * File: build_calculations.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <cmath>

#include "../support.h"
#include "../../utils/utils.h"


namespace MHWIBuildSearch
{


// Weapon base raw multiplied by this number is the raw cap.
static constexpr unsigned int k_RAW_CAP = 2;


static constexpr double k_RAW_BLUNDER_MULTIPLIER = 0.75;

static constexpr unsigned int k_POWERCHARM_RAW = 6;
static constexpr unsigned int k_POWERTALON_RAW = 9;


static double calculate_efr(unsigned int weapon_raw, // True raw, not bloated raw.
                            int          weapon_aff,
                            double       base_raw_multiplier,
                            unsigned int added_raw,
                            int          added_aff,
                            double       raw_crit_dmg_multiplier,
                            double       raw_sharpness_modifier) {

    assert(weapon_raw > 0);
    assert(base_raw_multiplier > 0.0);
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

    const unsigned int raw_cap = weapon_raw * k_RAW_CAP;

    const double weapon_multiplied_raw = weapon_raw * base_raw_multiplier;
    const unsigned int precap_true_raw = std::round(weapon_multiplied_raw) + added_raw;
    const unsigned int postcap_true_raw = std::min(precap_true_raw, raw_cap);
    // TODO: Find a way to output the wasted raw.
    
    const double efr = postcap_true_raw * raw_crit_modifier * raw_sharpness_modifier;
    return efr;
}


double calculate_efr_from_skills_lookup(const WeaponContribution& wc,
                                        const SkillMap&           full_skills,
                                        const SkillSpec&          skill_spec) {

    SkillContribution sc(full_skills, skill_spec, wc);
    return calculate_efr(wc.weapon_raw,
                         wc.weapon_aff,
                         sc.base_raw_multiplier,
                         sc.added_raw + k_POWERCHARM_RAW + k_POWERTALON_RAW,
                         sc.added_aff,
                         sc.raw_crit_dmg_multiplier,
                         sc.raw_sharpness_modifier);
}


double calculate_efr_from_gear_lookup(const WeaponInstance& weapon,
                                      const ArmourEquips&   armour,
                                      const DecoEquips&     decos,
                                      const SkillSpec&      skill_spec) {
    WeaponContribution wc = weapon.calculate_contribution();

    assert(decos.fits_in(armour, wc));

    SkillMap skills = armour.get_skills_without_set_bonuses();
    skills.merge_in(decos);
    if (wc.skill) skills.increment(wc.skill, 1);
    SetBonusMap set_bonuses = armour.get_set_bonuses();
    if (wc.set_bonus) set_bonuses.increment(wc.set_bonus, 1);
    skills.add_set_bonuses(set_bonuses);

    return calculate_efr_from_skills_lookup(wc, skills, skill_spec);
}


} // namespace

