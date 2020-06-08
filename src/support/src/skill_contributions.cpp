/*
 * File: skill_contributions.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <stdexcept>

#include "../../database/database_skills.h"
#include "../support.h"

namespace MHWIBuildSearch
{


// Affinity Sliding
static constexpr int k_AFFINITY_SLIDING_AFF = 30;

// Agitator                                             level: 0, 1, 2,  3,  4,  5,  6,  7
static const std::array<unsigned int, 8> agitator_added_raw = {0, 4, 8, 12, 16, 20, 24, 28};
static const std::array<int         , 8> agitator_added_aff = {0, 5, 5, 7,  7,  10, 15, 20};

// Airborne
static constexpr double k_AIRBORNE_RAW_MULTIPLIER = 1.3;

// Attack Boost                                             level: 0, 1, 2, 3,  4,  5,  6,  7
static const std::array<unsigned int, 8> attack_boost_added_raw = {0, 3, 6, 9, 12, 15, 18, 21};
static const std::array<int         , 8> attack_boost_added_aff = {0, 0, 0, 0,  5,  5,  5,  5};

// Bludgeoner
static constexpr unsigned int k_BLUDGEONER_ADDED_RAW_GREEN  = 15;
static constexpr unsigned int k_BLUDGEONER_ADDED_RAW_YELLOW = 25;
static constexpr unsigned int k_BLUDGEONER_ADDED_RAW_ORANGE = 30;
static constexpr unsigned int k_BLUDGEONER_ADDED_RAW_RED    = 30;
static constexpr unsigned int k_BLUDGEONER_ADDED_RAW_OTHER  = 0;

// Coalescence                                             level: 0,  1,  2,  3
static const std::array<unsigned int, 4> coalescence_added_raw = {0, 12, 15, 18};

// Critical Boost
static constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB0 = 1.25; // Critical Boost 0
static constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB1 = 1.30; // Critical Boost 1
static constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB2 = 1.35; // Critical Boost 2
static constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB3 = 1.40; // Critical Boost 3

// Critical Draw                                    level: 0,  1,  2,   3
static const std::array<int, 4> critical_draw_added_aff = {0, 30, 60, 100};

// Critical Eye                                    level: 0, 1,  2,  3,  4,  5,  6,  7
static const std::array<int, 8> critical_eye_added_aff = {0, 5, 10, 15, 20, 25, 30, 40};

// Dragonvein Awakening (and True Dragonvein Awakening)
static constexpr int k_DRAGONVEIN_AWAKENING_AFF      = 20;
static constexpr int k_TRUE_DRAGONVEIN_AWAKENING_AFF = 40;

// Fortify
static constexpr double k_FORTIFY_RAW_MULTIPLIER_S1 = 1.1;
static constexpr double k_FORTIFY_RAW_MULTIPLIER_S2 = 1.2;

// Frostcraft
// Table 1: Greatsword, Hammer                              state: 0     1     2     3
static const std::array<double, 4> frostcraft_multipliers_gs_h  = {1.00, 1.05, 1.15, 1.30};
// Table 2: Heavy Bowgun
static const std::array<double, 4> frostcraft_multipliers_hbg = {1.00, 1.10, 1.20, 1.30};
// Table 3: Everything Else
static const std::array<double, 4> frostcraft_multipliers_else = {1.00, 1.05, 1.20, 1.25};

// Heroics (and Heroics Secret)                       level: 0,    1,    2,    3,    4,    5,    6,    7
static const std::array<double, 8> heroics_raw_multiplier = {1.00, 1.00, 1.05, 1.05, 1.10, 1.15, 1.25, 1.40};

// Latent Power                                    level: 0,  1,  2,  3,  4,  5,  6,  7
static const std::array<int, 8> latent_power_added_aff = {0, 10, 20, 30, 40, 50, 50, 60};

// Maximum Might                                    level: 0,  1,  2,  3,  4,  5
static const std::array<int, 6> maximum_might_added_aff = {0, 10, 20, 30, 40, 40};

// Offensive Guard                                            level: 0,    1,    2,    3
static const std::array<double, 4> offensive_guard_raw_multiplier = {1.00, 1.05, 1.10, 1.15};

// Non-elemental Boost
static constexpr double k_NON_ELEMENTAL_BOOST_MULTIPLIER_DISABLED = 1.00;
static constexpr double k_NON_ELEMENTAL_BOOST_MULTIPLIER_ACTIVE   = 1.05;

// Peak Performance                                             level: 0, 1,  2,  3
static const std::array<unsigned int, 4> peak_performance_added_raw = {0, 5, 10, 20};

// Punishing Draw
static constexpr unsigned int k_PUNISHING_DRAW_ADDED_RAW = 5;

// Resentment                                             level: 0, 1,  2,  3,  4,  5
static const std::array<unsigned int, 6> resentment_added_raw = {0, 5, 10, 15, 20, 25};

// Weakness Exploit                                     level: 0,  1,  2,  3
static constexpr std::array<int, 4> weakness_exploit_s1_aff = {0, 10, 15, 30};
static constexpr std::array<int, 4> weakness_exploit_s2_aff = {0, 15, 30, 50};


static double calculate_non_elemental_boost_multiplier(const SkillMap& skills,
                                                       const SkillSpec& skills_spec,
                                                       const WeaponContribution& wc) {

    if (!skills.binary_skill_is_lvl1(&SkillsDatabase::g_skill_non_elemental_boost)) {
        return k_NON_ELEMENTAL_BOOST_MULTIPLIER_DISABLED;
    }

    // We can now assume the Non-elemental Boost skill is active.

    const bool is_raw = [&](){
        switch (wc.elestat_visibility) {
            case EleStatVisibility::none:
                return true;
            case EleStatVisibility::open:
                assert(wc.elestat_type != EleStatType::none);
                //assert(wc.elestat_value); // Do not make this assumption!
                return false;
            case EleStatVisibility::hidden:
                assert(wc.elestat_type != EleStatType::none);
                //assert(wc.elestat_value); // Do not make this assumption!
                if (skills.get(&SkillsDatabase::g_skill_free_elem_ammo_up) > 0) {
                    return false;
                }
                if ((!skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_element_acceleration))
                        && (!skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_true_element_acceleration))) {
                    return true;
                }
                return skills.binary_skill_is_lvl1(&SkillsDatabase::g_skill_element_acceleration)
                       || skills.binary_skill_is_lvl1(&SkillsDatabase::g_skill_true_element_acceleration);
            default:
                throw std::logic_error("Unexpected EleStatVisibility value.");
        }
    }();

    return (is_raw) ? k_NON_ELEMENTAL_BOOST_MULTIPLIER_ACTIVE : k_NON_ELEMENTAL_BOOST_MULTIPLIER_DISABLED;
}


static double calculate_raw_crit_dmg_multiplier(const SkillMap& skills) {
    switch (skills.get(&SkillsDatabase::g_skill_critical_boost)) {
        case 0: return k_RAW_CRIT_DMG_MULTIPLIER_CB0;
        case 1: return k_RAW_CRIT_DMG_MULTIPLIER_CB1;
        case 2: return k_RAW_CRIT_DMG_MULTIPLIER_CB2;
        default:
            assert(skills.get(&SkillsDatabase::g_skill_critical_boost) == 3);
            return k_RAW_CRIT_DMG_MULTIPLIER_CB3;
    }
}


static SharpnessGauge calculate_final_sharpness_gauge(const SkillMap& skills,
                                                      const WeaponContribution& wc) {
    if (wc.is_constant_sharpness) {
        return wc.maximum_sharpness;
    } else {
        const unsigned int handicraft_lvl = skills.get(&SkillsDatabase::g_skill_handicraft);
        return wc.maximum_sharpness.apply_handicraft(handicraft_lvl);
    }
}


SkillContribution::SkillContribution(const SkillMap&           skills,
                                     const SkillSpec&          skills_spec,
                                     const WeaponClass         weapon_class,
                                     const WeaponContribution& wc ) noexcept
    : added_raw                 (0)
    , added_aff                 (0)
    , base_raw_multiplier       (calculate_non_elemental_boost_multiplier(skills, skills_spec, wc))
    //, frostcraft_raw_multiplier (1.0) // We initialize later
    //, bludgeoner_added_raw      (0)   // We initialize later
    , raw_crit_dmg_multiplier   (calculate_raw_crit_dmg_multiplier(skills))
    , final_sharpness_gauge     (calculate_final_sharpness_gauge(skills, wc))
{
    // We calculate the remaining fields.

    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_affinity_sliding)
            && skills.binary_skill_is_lvl1(&SkillsDatabase::g_skill_affinity_sliding)) {
        this->added_aff += k_AFFINITY_SLIDING_AFF;
    }

    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_agitator)) {
        const unsigned int agitator_lvl = skills.get_non_secret(&SkillsDatabase::g_skill_agitator,
                                                                &SkillsDatabase::g_skill_agitator_secret);
        this->added_raw += agitator_added_raw[agitator_lvl];
        this->added_aff += agitator_added_aff[agitator_lvl];
    }

    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_airborne)
            && skills.binary_skill_is_lvl1(&SkillsDatabase::g_skill_airborne)) {
        this->base_raw_multiplier *= k_AIRBORNE_RAW_MULTIPLIER;
    }

    const unsigned int attack_boost_lvl = skills.get(&SkillsDatabase::g_skill_attack_boost);
    this->added_raw += attack_boost_added_raw[attack_boost_lvl];
    this->added_aff += attack_boost_added_aff[attack_boost_lvl];

    if (skills.binary_skill_is_lvl1(&SkillsDatabase::g_skill_bludgeoner)) {
        switch (this->final_sharpness_gauge.get_sharpness_level()) {
            case SharpnessLevel::red:
                this->bludgeoner_added_raw = k_BLUDGEONER_ADDED_RAW_RED;
                break;
            case SharpnessLevel::orange:
                this->bludgeoner_added_raw = k_BLUDGEONER_ADDED_RAW_ORANGE;
                break;
            case SharpnessLevel::yellow:
                this->bludgeoner_added_raw = k_BLUDGEONER_ADDED_RAW_YELLOW;
                break;
            case SharpnessLevel::green:
                this->bludgeoner_added_raw = k_BLUDGEONER_ADDED_RAW_GREEN;
                break;
            default:
                this->bludgeoner_added_raw = k_BLUDGEONER_ADDED_RAW_OTHER;
        }
    } else {
        this->bludgeoner_added_raw = k_BLUDGEONER_ADDED_RAW_OTHER;
    }

    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_coalescence)) {
        const unsigned int coalescence_lvl = skills.get(&SkillsDatabase::g_skill_coalescence);
        this->added_raw += coalescence_added_raw[coalescence_lvl];
    }

    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_critical_draw)) {
        const unsigned int critical_draw_lvl = skills.get(&SkillsDatabase::g_skill_critical_draw);
        this->added_aff += critical_draw_added_aff[critical_draw_lvl];
    }

    const unsigned int critical_eye_lvl = skills.get(&SkillsDatabase::g_skill_critical_eye);
    this->added_aff += critical_eye_added_aff[critical_eye_lvl];

    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_dragonvein_awakening)
            || skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_true_dragonvein_awakening)) {
        if (skills.binary_skill_is_lvl1(&SkillsDatabase::g_skill_true_dragonvein_awakening)) {
            this->added_aff += k_TRUE_DRAGONVEIN_AWAKENING_AFF;
        } else if (skills.binary_skill_is_lvl1(&SkillsDatabase::g_skill_dragonvein_awakening)) {
            this->added_aff += k_DRAGONVEIN_AWAKENING_AFF;
        }
    }

    if (skills.binary_skill_is_lvl1(&SkillsDatabase::g_skill_fortify)) {
        const unsigned int fortify_state = skills_spec.get_state(&SkillsDatabase::g_skill_fortify);
        switch (fortify_state) {
            case 1:
                this->base_raw_multiplier *= k_FORTIFY_RAW_MULTIPLIER_S1;
                break;
            case 2:
                this->base_raw_multiplier *= k_FORTIFY_RAW_MULTIPLIER_S2;
                break;
            default:
                assert(fortify_state == 0);
        }
    }

    if (skills.binary_skill_is_lvl1(&SkillsDatabase::g_skill_frostcraft)) {
        const unsigned int frostcraft_state = skills_spec.get_state(&SkillsDatabase::g_skill_frostcraft);
        //assert(frostcraft_state > 0); // TODO: Should we add an if-statement for the zero-state?
        assert(frostcraft_state < frostcraft_multipliers_gs_h.size());
        assert(frostcraft_state < frostcraft_multipliers_hbg.size());
        assert(frostcraft_state < frostcraft_multipliers_else.size());
        switch (weapon_class) {
            case WeaponClass::greatsword:
            case WeaponClass::hammer:
                this->frostcraft_raw_multiplier = frostcraft_multipliers_gs_h[frostcraft_state];
                break;
            case WeaponClass::heavy_bowgun:
                this->frostcraft_raw_multiplier = frostcraft_multipliers_hbg[frostcraft_state];
                break;
            default:
                this->frostcraft_raw_multiplier = frostcraft_multipliers_else[frostcraft_state];
        }
    } else {
        this->frostcraft_raw_multiplier = 1.0;
    }

    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_heroics)) {
        const unsigned int heroics_lvl = skills.get_non_secret(&SkillsDatabase::g_skill_heroics,
                                                               &SkillsDatabase::g_skill_heroics_secret);
        this->base_raw_multiplier *= heroics_raw_multiplier[heroics_lvl];
    }

    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_latent_power)) {
        const unsigned int latent_power_lvl = skills.get_non_secret(&SkillsDatabase::g_skill_latent_power,
                                                                    &SkillsDatabase::g_skill_latent_power_secret);
        this->added_aff += latent_power_added_aff[latent_power_lvl];
    }

    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_maximum_might)) {
        const unsigned int maximum_might_lvl = skills.get_non_secret(&SkillsDatabase::g_skill_maximum_might,
                                                                     &SkillsDatabase::g_skill_maximum_might_secret);
        this->added_aff += maximum_might_added_aff[maximum_might_lvl];
    }

    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_offensive_guard)) {
        const unsigned int offensive_guard_lvl = skills.get(&SkillsDatabase::g_skill_offensive_guard);
        this->base_raw_multiplier *= offensive_guard_raw_multiplier[offensive_guard_lvl];
    }

    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_peak_performance)) {
        const unsigned int peak_performance_lvl = skills.get(&SkillsDatabase::g_skill_peak_performance);
        this->added_raw += peak_performance_added_raw[peak_performance_lvl];
    }

    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_punishing_draw)
            && skills.binary_skill_is_lvl1(&SkillsDatabase::g_skill_punishing_draw)) {
        this->added_raw += k_PUNISHING_DRAW_ADDED_RAW;
    }

    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_resentment)) {
        const unsigned int resentment_lvl = skills.get(&SkillsDatabase::g_skill_resentment);
        this->added_raw += resentment_added_raw[resentment_lvl];
    }

    const unsigned int wex_state = skills_spec.get_state(&SkillsDatabase::g_skill_weakness_exploit);
    assert(SkillsDatabase::g_skill_weakness_exploit.states == 3);
    assert(wex_state <= 2);
    if (wex_state == 1) {
        const unsigned int wex_lvl = skills.get(&SkillsDatabase::g_skill_weakness_exploit);
        this->added_aff += weakness_exploit_s1_aff[wex_lvl];
    } else if (wex_state == 2) {
        const unsigned int wex_lvl = skills.get(&SkillsDatabase::g_skill_weakness_exploit); // Redundant?
        this->added_aff += weakness_exploit_s2_aff[wex_lvl];
    }
}


} // namespace

