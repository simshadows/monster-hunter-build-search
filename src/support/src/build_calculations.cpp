/*
 * File: build_calculations.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <stdexcept>
#include <cmath>

#include "../support.h"
#include "../../utils/utils.h"


namespace MHWIBuildSearch
{


// Weapon base raw multiplied by this number is the raw cap.
static constexpr unsigned int k_RAW_CAP = 2;

static constexpr double k_NORMAL_STATUS_APPL_PROBABILITY = 1.0/3;

static constexpr double k_RAW_BLUNDER_MULTIPLIER = 0.75;


static EffectiveDamageValues calculate_edv(const unsigned int    weapon_raw, // True raw, not bloated raw.
                                           const int             weapon_aff,

                                           const EleStatVisibility weapon_elestat_visibility,
                                           const EleStatType       weapon_elestat_type,
                                           const unsigned int      weapon_elestat_value,

                                           const double          base_raw_multiplier,
                                           const double          frostcraft_raw_multiplier,
                                           const unsigned int    bludgeoner_added_raw,
                                           const unsigned int    added_raw,
                                           const int             added_aff,
                                           const double          raw_crit_dmg_multiplier,
                                           const SharpnessGauge& final_sharpness_gauge,

                                           const unsigned int    free_element_active_percentage ) {

    assert(weapon_raw > 0);
    assert(base_raw_multiplier > 0.0);
    assert(raw_crit_dmg_multiplier > 0.0);

    const int affinity = weapon_aff + added_aff;

    const double raw_crit_modifier = [&](){
        //double raw_crit_chance = std::clamp(((double) (weapon_aff + added_aff)) / 100, -1.0, 1.0); // Could be better
        double raw_crit_chance = ((double) affinity) / 100;
        if (raw_crit_chance < 0) {
            double raw_blunder_chance = (raw_crit_chance < -1.0) ? -1.0 : -raw_crit_chance;
            return (k_RAW_BLUNDER_MULTIPLIER * raw_blunder_chance) + (1 - raw_blunder_chance);
        } else {
            raw_crit_chance = (raw_crit_chance > 1.0) ? 1.0 : raw_crit_chance;
            return (raw_crit_dmg_multiplier * raw_crit_chance) + (1 - raw_crit_chance);
        }
    }();

    /*
     * Effective Raw
     */

    const double raw_sharpness_modifier = final_sharpness_gauge.get_raw_sharpness_modifier();

    const unsigned int raw_cap = weapon_raw * k_RAW_CAP;

    const double weapon_multiplied_raw = (weapon_raw + bludgeoner_added_raw) * base_raw_multiplier;
    const unsigned int precap_true_raw = std::round(weapon_multiplied_raw) + added_raw;
    const unsigned int postcap_true_raw = std::min(precap_true_raw, raw_cap);
    
    const double efr = postcap_true_raw * raw_crit_modifier * raw_sharpness_modifier * frostcraft_raw_multiplier;

    /*
     * Effective Element/Status
     */

    const double elestat_combined_modifier = [&](){
        if (weapon_elestat_visibility == EleStatVisibility::none) {
            return 1.0; // Disabled modifier
        } else if (elestattype_is_element(weapon_elestat_type)) {
            // Weapon is elemental.
            // We just return the sharpness modifier.
            return final_sharpness_gauge.get_elemental_sharpness_modifier();
        } else {
            // Weapon is status.
            // We will need to factor in average status proc probability.
            return k_NORMAL_STATUS_APPL_PROBABILITY;
        }
    }();

    const unsigned int base_elestat_value = [&](){
        if (weapon_elestat_visibility == EleStatVisibility::hidden) {
            return (weapon_elestat_value * free_element_active_percentage) / 100;
        } else {
            assert((weapon_elestat_visibility == EleStatVisibility::open)
                   || ((weapon_elestat_visibility == EleStatVisibility::none) && (!weapon_elestat_value)));
            return weapon_elestat_value;
        }
    }();

    const double efes = base_elestat_value * elestat_combined_modifier;
    const EleStatType elestat_type = weapon_elestat_type;

    return {affinity,
            final_sharpness_gauge,
            ((double) precap_true_raw / raw_cap),
            efr,

            efes,
            elestat_type };
}


EffectiveDamageValues calculate_edv_from_skills_lookup(const WeaponClass         weapon_class,
                                                       const WeaponContribution& wc,
                                                       const SkillMap&           full_skills,
                                                       const MiscBuffsEquips&    misc_buffs,
                                                       const SkillSpec&          skill_spec) {

    SkillContribution sc(full_skills, skill_spec, weapon_class, wc);
    return calculate_edv(wc.weapon_raw,
                         wc.weapon_aff,

                         wc.elestat_visibility,
                         wc.elestat_type,
                         wc.elestat_value,

                         sc.base_raw_multiplier * misc_buffs.get_base_raw_multiplier(),
                         sc.frostcraft_raw_multiplier,
                         sc.bludgeoner_added_raw,
                         sc.added_raw + misc_buffs.get_added_raw(),
                         sc.added_aff,
                         sc.raw_crit_dmg_multiplier,
                         sc.final_sharpness_gauge,

                         sc.free_element_active_percentage );
}


EffectiveDamageValues calculate_edv_from_gear_lookup(const WeaponInstance&  weapon,
                                                     const ArmourEquips&    armour,
                                                     const DecoEquips&      decos,
                                                     const MiscBuffsEquips& misc_buffs,
                                                     const SkillSpec&       skill_spec) {
    WeaponContribution wc = weapon.calculate_contribution();

    assert(decos.fits_in(armour, wc));

    SkillMap skills = armour.get_skills_without_set_bonuses();
    skills.merge_in(decos);
    if (wc.skill) skills.increment(wc.skill, 1);
    SetBonusMap set_bonuses = armour.get_set_bonuses();
    if (wc.set_bonus) set_bonuses.increment(wc.set_bonus, 1);
    skills.add_set_bonuses(set_bonuses);

    return calculate_edv_from_skills_lookup(weapon.weapon->weapon_class, wc, skills, misc_buffs, skill_spec);
}


std::string EffectiveDamageValues::get_humanreadable() const {
    return "EFR: " + std::to_string(this->efr)
           + "\nEFE/EFS: " + std::to_string(this->efes) + " " + elestattype_to_str(this->elestat_type)
           + "\nAffinity: " + std::to_string(this->affinity)
           + "\nSharpness Gauge: " + this->final_sharpness_gauge.get_humanreadable()
           + "\nPre-Raw Cap Ratio: " + std::to_string(this->pre_raw_cap_ratio * 100) + "%";
}


static double calculate_poison_damage(const DamageModel& md,
                                      const double raw_damage_per_iter) {
    const double total_poison_damage = ((double)md.poison_proc_dmg) * md.poison_total_procs;
    return (raw_damage_per_iter * total_poison_damage) / (((double)md.target_health) - total_poison_damage);
}


// raw_damage_per_iter can be raw damage per hit
static double calculate_blast_damage(const DamageModel& md,
                                     const EffectiveDamageValues& edv,
                                     const double raw_damage_per_iter) {

    // The variable names here will reflect the variable names used in the blast damage model document
    // at (<{REPOSITORY_ROOT}/docs/blast_damage_model/blast_damage_model.pdf>).

    // This implementation is the simplified "continuous model".

    if (!edv.efes) {
        return 0.0; // Zero blast damage because we never build status
    }

    // Player attack parameters
    const double rho   = raw_damage_per_iter;
    const double sigma = edv.efes * md.status_modifier;
    assert(rho);
    assert(sigma);

    // Target misc parameters
    const double cap_h = md.target_health;
    assert(cap_h);

    // Target blast parameters
    const double a1    = md.blast_base;
    const double d     = md.blast_buildup;
    const double c     = md.blast_cap;
    const double cap_p = md.blast_proc_dmg;
    assert(a1);
    assert(d);
    assert(c);
    assert(cap_p);

    // Anonymous variables.
    const double cap_z = (std::pow(cap_p,2) * sigma) / (rho*d);
    const double cap_y = (cap_p / (2*d)) * ( ((2*a1 - d) / cap_p) + (2*sigma / rho) );
    const double cap_x = (rho / (2*sigma*d)) * (std::pow(c,2) + (c*d) + (a1*d) - std::pow(a1,2)) - (cap_p*cap_y);

    // cap_c is the maximum amount of health before reaching the blast cap.
    const double cap_c = cap_z + std::sqrt(std::pow(cap_y,2) + std::pow(cap_x,2) + std::pow(cap_z,2));
    assert(cap_c);

    // cap_b is a function of health to total blast damage.
    const auto cap_b = [&](const double tmp_h){return std::sqrt((2*tmp_h*cap_z) + std::pow(cap_y,2)) - (cap_p*cap_y);};
    const auto beta_p = [&](const double tmp_h){const double eval_cap_b = cap_b(cap_h);
                                                assert(eval_cap_b);
                                                return rho * (eval_cap_b / (tmp_h - eval_cap_b)); };

    if (cap_h <= cap_c) {
        // We don't reach blast cap
        const double ret = beta_p(cap_h);
        assert(ret);
        return ret;
    } else {
        // We must now account for blast cap
        const double ret = (cap_c/cap_h) * (rho+beta_p(cap_c)) + ((cap_h-cap_c)/cap_h) * (rho + ((sigma*cap_p)/c)) - rho;
        assert(ret);
        return ret;
    }
}


// TODO: Implement the special rounding function that implements special handling of values between
//       -1.0 and 1.0 to always round away from zero.
ModelCalculatedValues calculate_damage(const DamageModel& model,
                                       const EffectiveDamageValues& edv) {

    const double unrounded_raw_damage = (edv.efr / 100) * (double) model.raw_motion_value * ((double) model.hzv_raw / 100);

    const double unrounded_elestat_damage = [&](){
        if (edv.efes == 0) {
            return 0.0; // Zero EFE/EFS always means zero damage.
        } else if (elestattype_is_element(edv.elestat_type)) {
            assert(edv.elestat_type != EleStatType::none);
            const double ele_hzv = [&](){
                switch (edv.elestat_type) {
                    case EleStatType::fire:    return (double) model.hzv_fire    / 100;
                    case EleStatType::water:   return (double) model.hzv_water   / 100;
                    case EleStatType::thunder: return (double) model.hzv_thunder / 100;
                    case EleStatType::ice:     return (double) model.hzv_ice     / 100;
                    case EleStatType::dragon:  return (double) model.hzv_dragon  / 100;
                    default:
                        throw std::logic_error("Unexpected EleStatType value. Expected an elemental.");
                }
            }();
            return model.elemental_modifier * edv.efes * ele_hzv;
        } else {
            switch (edv.elestat_type) {
                case EleStatType::poison:
                    return calculate_poison_damage(model, unrounded_raw_damage);
                case EleStatType::paralysis:
                case EleStatType::sleep:
                    return 0.0; // Sleep and paralysis do no damage.
                case EleStatType::blast:
                    return calculate_blast_damage(model, edv, unrounded_raw_damage);
                default:
                    throw std::logic_error("Unexpected EleStatType value. Expected a status.");
            }
        }
    }();

    assert(unrounded_raw_damage >= 0.0);
    assert(unrounded_elestat_damage >= 0.0);

    const double unrounded_total_damage = unrounded_raw_damage + unrounded_elestat_damage;

    // TODO: Damage from status effects shouldn't be rounded. Fix this.
    const unsigned int actual_total_damage = std::round(unrounded_raw_damage) + std::round(unrounded_elestat_damage);

    return {unrounded_raw_damage,
            unrounded_elestat_damage,
            unrounded_total_damage,
            actual_total_damage };
}


std::string DamageModel::get_humanreadable() const {
    return "Raw Motion Value:   " + std::to_string(this->raw_motion_value)
           + "\nElemental Modifier: " + std::to_string(this->elemental_modifier)
           + "\n"
           + "\nRaw HZV:     " + std::to_string(this->hzv_raw)
           + "\nFire HZV:    " + std::to_string(this->hzv_fire)
           + "\nWater HZV:   " + std::to_string(this->hzv_water)
           + "\nThunder HZV: " + std::to_string(this->hzv_thunder)
           + "\nIce HZV:     " + std::to_string(this->hzv_ice)
           + "\nDragon HZV:  " + std::to_string(this->hzv_dragon)
           + "\n"
           + "\nPoison Total Procs Per Quest: " + std::to_string(this->poison_total_procs)
           + "\nPoison Damage Per Proc:       " + std::to_string(this->poison_proc_dmg)
           + "\n"
           + "\nBlast Base:        " + std::to_string(this->blast_base)
           + "\nBlast Buildup:     " + std::to_string(this->blast_buildup)
           + "\nBlast Cap:         " + std::to_string(this->blast_cap)
           + "\nBlast Proc Damage: " + std::to_string(this->blast_proc_dmg)
           + "\n"
           + "\nTarget Health: " + std::to_string(this->target_health);
}


std::string ModelCalculatedValues::get_humanreadable() const {
    return "Unrounded Raw Damage: " + std::to_string(this->unrounded_raw_damage)
           + "\nUnrounded Elemental/Status Damage: " + std::to_string(this->unrounded_elestat_damage)
           + "\nUnrounded Total Damage: " + std::to_string(this->unrounded_total_damage)
           + "\nActual Total Damage: " + std::to_string(this->actual_total_damage);
}


} // namespace

