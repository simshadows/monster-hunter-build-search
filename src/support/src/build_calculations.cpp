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


static EffectiveDamageValues calculate_edv(const unsigned int    weapon_raw, // True raw, not bloated raw.
                                           const int             weapon_aff,
                                           const double          base_raw_multiplier,
                                           const double          frostcraft_raw_multiplier,
                                           const unsigned int    bludgeoner_added_raw,
                                           const unsigned int    added_raw,
                                           const int             added_aff,
                                           const double          raw_crit_dmg_multiplier,
                                           const SharpnessGauge& final_sharpness_gauge) {

    assert(weapon_raw > 0);
    assert(base_raw_multiplier > 0.0);
    assert(raw_crit_dmg_multiplier > 0.0);

    const double raw_sharpness_modifier = final_sharpness_gauge.get_raw_sharpness_modifier();
    const int affinity = weapon_aff + added_aff;

    //double raw_crit_chance = std::clamp(((double) (weapon_aff + added_aff)) / 100, -1.0, 1.0); // C++17
    double raw_crit_chance = ((double) affinity) / 100;
    double raw_crit_modifier;
    if (raw_crit_chance < 0) {
        double raw_blunder_chance = (raw_crit_chance < -1.0) ? -1.0 : -raw_crit_chance;
        raw_crit_modifier = (k_RAW_BLUNDER_MULTIPLIER * raw_blunder_chance) + (1 - raw_blunder_chance);
    } else {
        raw_crit_chance = (raw_crit_chance > 1.0) ? 1.0 : raw_crit_chance;
        raw_crit_modifier = (raw_crit_dmg_multiplier * raw_crit_chance) + (1 - raw_crit_chance);
    }

    const unsigned int raw_cap = weapon_raw * k_RAW_CAP;

    const double weapon_multiplied_raw = (weapon_raw + bludgeoner_added_raw) * base_raw_multiplier;
    const unsigned int precap_true_raw = std::round(weapon_multiplied_raw) + added_raw;
    const unsigned int postcap_true_raw = std::min(precap_true_raw, raw_cap);
    
    const double efr = postcap_true_raw * raw_crit_modifier * raw_sharpness_modifier * frostcraft_raw_multiplier;

    return {affinity,
            final_sharpness_gauge,
            ((double) precap_true_raw / raw_cap),
            efr };
}


EffectiveDamageValues calculate_edv_from_skills_lookup(const WeaponClass         weapon_class,
                                                       const WeaponContribution& wc,
                                                       const SkillMap&           full_skills,
                                                       const SkillSpec&          skill_spec) {

    SkillContribution sc(full_skills, skill_spec, weapon_class, wc);
    return calculate_edv(wc.weapon_raw,
                         wc.weapon_aff,
                         sc.base_raw_multiplier,
                         sc.frostcraft_raw_multiplier,
                         sc.bludgeoner_added_raw,
                         sc.added_raw + k_POWERCHARM_RAW + k_POWERTALON_RAW,
                         sc.added_aff,
                         sc.raw_crit_dmg_multiplier,
                         sc.final_sharpness_gauge);
}


EffectiveDamageValues calculate_edv_from_gear_lookup(const WeaponInstance& weapon,
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

    return calculate_edv_from_skills_lookup(weapon.weapon->weapon_class, wc, skills, skill_spec);
}


std::string EffectiveDamageValues::get_humanreadable() const {
    return "EFR: " + std::to_string(this->efr)
           + "\nAffinity: " + std::to_string(this->affinity)
           + "\nSharpness Gauge: " + this->final_sharpness_gauge.get_humanreadable()
           + "\nPre-Raw Cap Ratio: " + std::to_string(this->pre_raw_cap_ratio * 100) + "%";
}


} // namespace

