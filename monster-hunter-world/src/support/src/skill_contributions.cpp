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
static constexpr int k_TRUE_DRAGONVEIN_AWAKENING_AFF = 20; // This adds on top of the non-True version.

// Element Acceleration (and True Element Acceleration)
static constexpr unsigned int k_ELEMENT_ACCELERATION_FREEELEM_LVL      = 2;
static constexpr unsigned int k_TRUE_ELEMENT_ACCELERATION_FREEELEM_LVL = 1; // This adds on top of the non-True version.

// Fortify
static constexpr double k_FORTIFY_RAW_MULTIPLIER_S1 = 1.1;
static constexpr double k_FORTIFY_RAW_MULTIPLIER_S2 = 1.2;

// Free Element                                                          level: 0, 1,  2,  3
static const std::array<unsigned int, 4> free_element_active_percentage_vals = {0, 33, 66, 100};

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
static constexpr double k_NON_ELEMENTAL_BOOST_MULTIPLIER = 1.05;

// Peak Performance                                             level: 0, 1,  2,  3
static const std::array<unsigned int, 4> peak_performance_added_raw = {0, 5, 10, 20};

// Punishing Draw
static constexpr unsigned int k_PUNISHING_DRAW_ADDED_RAW = 5;

// Resentment                                             level: 0, 1,  2,  3,  4,  5
static const std::array<unsigned int, 6> resentment_added_raw = {0, 5, 10, 15, 20, 25};

// Weakness Exploit                                     level: 0,  1,  2,  3
static constexpr std::array<int, 4> weakness_exploit_s1_aff = {0, 10, 15, 30};
static constexpr std::array<int, 4> weakness_exploit_s2_aff = {0, 15, 30, 50};


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
    , base_raw_multiplier       (1.0)
    , frostcraft_raw_multiplier (1.0)
    , bludgeoner_added_raw      (0)
    , raw_crit_dmg_multiplier   (k_RAW_CRIT_DMG_MULTIPLIER_CB0)
    , final_sharpness_gauge     (calculate_final_sharpness_gauge(skills, wc))
    //, free_element_active_percentage (0) // We will initialize this later!
{
    // We calculate the remaining fields.

    bool non_elemental_boost_is_present = false;
    unsigned int effective_free_element_lvl = 0;

    for (const auto& skill_pair : skills) {
        const Skill * const skill = skill_pair.first;
        const unsigned int lvl = skill_pair.second;

        // We guarantee that skills we see here have at least one level.
        assert(lvl);

        // This function will return the actual effective level, depending on whether the "secret skill" is active.
        const auto apply_secret = [&](const Skill * const associated_secret){
            if ((lvl <= skill->normal_limit) || skills.binary_skill_is_lvl1(associated_secret)) {
                return lvl;
            } else {
                return skill->normal_limit;
            }
        };

        switch (skill->nid) {

            case SkillsDatabase::g_skillnid_affinity_sliding: {
                    assert(lvl == 1);
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_affinity_sliding)) {
                        this->added_aff += k_AFFINITY_SLIDING_AFF;
                    }
                } break;

            case SkillsDatabase::g_skillnid_agitator: {
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_agitator)) {
                        const unsigned int effective_lvl = apply_secret(&SkillsDatabase::g_skill_agitator_secret);
                        this->added_raw += agitator_added_raw[effective_lvl];
                        this->added_aff += agitator_added_aff[effective_lvl];
                    }
                } break;

            case SkillsDatabase::g_skillnid_airborne: {
                    assert(lvl == 1);
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_airborne)) {
                        this->base_raw_multiplier *= k_AIRBORNE_RAW_MULTIPLIER;
                    }
                } break;

            case SkillsDatabase::g_skillnid_attack_boost: {
                    this->added_raw += attack_boost_added_raw[lvl];
                    this->added_aff += attack_boost_added_aff[lvl];
                } break;

            case SkillsDatabase::g_skillnid_bludgeoner: {
                    assert(lvl == 1);
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
                } break;

            case SkillsDatabase::g_skillnid_coalescence: {
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_coalescence)) {
                        this->added_raw += coalescence_added_raw[lvl];
                    }
                } break;

            case SkillsDatabase::g_skillnid_critical_boost: {
                    switch (lvl) {
                        case 1:
                            this->raw_crit_dmg_multiplier = k_RAW_CRIT_DMG_MULTIPLIER_CB1;
                            break;
                        case 2:
                            this->raw_crit_dmg_multiplier = k_RAW_CRIT_DMG_MULTIPLIER_CB2;
                            break;
                        default:
                            assert(lvl == 3);
                            this->raw_crit_dmg_multiplier = k_RAW_CRIT_DMG_MULTIPLIER_CB3;
                    }
                } break;

            case SkillsDatabase::g_skillnid_critical_draw: {
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_critical_draw)) {
                        this->added_aff += critical_draw_added_aff[lvl];
                    }
                } break;

            case SkillsDatabase::g_skillnid_critical_eye: {
                    this->added_aff += critical_eye_added_aff[lvl];
                } break;

            case SkillsDatabase::g_skillnid_dragonvein_awakening: {
                    assert(lvl == 1);
                    assert(skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_dragonvein_awakening)
                           == skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_true_dragonvein_awakening));
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_dragonvein_awakening)) {
                        this->added_aff += k_DRAGONVEIN_AWAKENING_AFF;
                    }
                } break;
            case SkillsDatabase::g_skillnid_true_dragonvein_awakening: {
                    assert(lvl == 1);
                    assert(skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_dragonvein_awakening)
                           == skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_true_dragonvein_awakening));
                    assert(skills.get(&SkillsDatabase::g_skill_dragonvein_awakening) == 1);
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_dragonvein_awakening)) {
                        this->added_aff += k_TRUE_DRAGONVEIN_AWAKENING_AFF;
                    }
                } break;

            case SkillsDatabase::g_skillnid_element_acceleration: {
                    assert(lvl == 1);
                    assert(skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_element_acceleration)
                           == skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_true_element_acceleration));
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_element_acceleration)) {
                        effective_free_element_lvl += k_ELEMENT_ACCELERATION_FREEELEM_LVL;
                    }
                } break;
            case SkillsDatabase::g_skillnid_true_element_acceleration: {
                    assert(lvl == 1);
                    assert(skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_element_acceleration)
                           == skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_true_element_acceleration));
                    assert(skills.get(&SkillsDatabase::g_skill_element_acceleration) == 1);
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_true_element_acceleration)) {
                        effective_free_element_lvl += k_TRUE_ELEMENT_ACCELERATION_FREEELEM_LVL;
                    }
                } break;

            case SkillsDatabase::g_skillnid_free_elem_ammo_up: {
                    effective_free_element_lvl += lvl;
                } break;

            case SkillsDatabase::g_skillnid_fortify: {
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
                } break;

            case SkillsDatabase::g_skillnid_frostcraft: {
                    assert(lvl == 1);
                    const unsigned int frostcraft_state = skills_spec.get_state(&SkillsDatabase::g_skill_frostcraft);
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
                } break;

            case SkillsDatabase::g_skillnid_heroics: {
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_heroics)) {
                        const unsigned int effective_lvl = apply_secret(&SkillsDatabase::g_skill_heroics_secret);
                        this->base_raw_multiplier *= heroics_raw_multiplier[effective_lvl];
                    }
                } break;

            case SkillsDatabase::g_skillnid_latent_power: {
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_latent_power)) {
                        const unsigned int effective_lvl = apply_secret(&SkillsDatabase::g_skill_latent_power_secret);
                        this->added_aff += latent_power_added_aff[effective_lvl];
                    }
                } break;

            case SkillsDatabase::g_skillnid_maximum_might: {
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_maximum_might)) {
                        const unsigned int effective_lvl = apply_secret(&SkillsDatabase::g_skill_maximum_might_secret);
                        this->added_aff += maximum_might_added_aff[effective_lvl];
                    }
                } break;

            case SkillsDatabase::g_skillnid_non_elemental_boost: {
                    non_elemental_boost_is_present = true;
                } break;

            case SkillsDatabase::g_skillnid_offensive_guard: {
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_offensive_guard)) {
                        this->base_raw_multiplier *= offensive_guard_raw_multiplier[lvl];
                    }
                } break;

            case SkillsDatabase::g_skillnid_peak_performance: {
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_peak_performance)) {
                        this->added_raw += peak_performance_added_raw[lvl];
                    }
                } break;

            case SkillsDatabase::g_skillnid_punishing_draw: {
                    assert(lvl == 1);
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_punishing_draw)) {
                        this->added_raw += k_PUNISHING_DRAW_ADDED_RAW;
                    }
                } break;

            case SkillsDatabase::g_skillnid_resentment: {
                    if (skills_spec.get_state_for_binary_skill(&SkillsDatabase::g_skill_resentment)) {
                        this->added_raw += resentment_added_raw[lvl];
                    }
                } break;

            case SkillsDatabase::g_skillnid_weakness_exploit: {
                    const unsigned int wex_state = skills_spec.get_state(&SkillsDatabase::g_skill_weakness_exploit);
                    assert(SkillsDatabase::g_skill_weakness_exploit.states == 3);
                    assert(wex_state <= 2);
                    if (wex_state == 1) {
                        this->added_aff += weakness_exploit_s1_aff[lvl];
                    } else if (wex_state == 2) {
                        this->added_aff += weakness_exploit_s2_aff[lvl];
                    }
                } break;

            default:
                ; // Do nothing
        }
    }

    if (effective_free_element_lvl) {
        // Clip the level
        effective_free_element_lvl = (effective_free_element_lvl > 3) ? 3 : effective_free_element_lvl;
        // Calculate the final value
        this->free_element_active_percentage = free_element_active_percentage_vals[effective_free_element_lvl];
    } else {
        this->free_element_active_percentage = 0;
        // Since we know that Free Element is not active, we just need to test if the element is open.
        if (wc.elestat_visibility != EleStatVisibility::open) {
            this->base_raw_multiplier *= k_NON_ELEMENTAL_BOOST_MULTIPLIER;
        }
    }

    (void)non_elemental_boost_is_present; // Silence compiler warning temporarily
}


} // namespace

