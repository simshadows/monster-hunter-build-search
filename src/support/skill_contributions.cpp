/*
 * File: skill_contributions.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include "support.h"

namespace MHWIBuildSearch
{


// Affinity Sliding
static constexpr int k_AFFINITY_SLIDING_AFF = 30;

// Agitator                                             level: 0, 1, 2,  3,  4,  5,  6,  7
static const std::array<unsigned int, 8> agitator_added_raw = {0, 4, 8, 12, 16, 20, 24, 28};
static const std::array<int         , 8> agitator_added_aff = {0, 5, 5, 7,  7,  10, 15, 20};

// Attack Boost                                             level: 0, 1, 2, 3,  4,  5,  6,  7
static const std::array<unsigned int, 8> attack_boost_added_raw = {0, 3, 6, 9, 12, 15, 18, 21};
static const std::array<int         , 8> attack_boost_added_aff = {0, 0, 0, 0,  5,  5,  5,  5};

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

// Latent Power                                    level: 0,  1,  2,  3,  4,  5,  6,  7
static const std::array<int, 8> latent_power_added_aff = {0, 10, 20, 30, 40, 50, 50, 60};

// Maximum Might                                    level: 0,  1,  2,  3,  4,  5
static const std::array<int, 6> maximum_might_added_aff = {0, 10, 20, 30, 40, 40};

// Non-elemental Boost
static constexpr double k_NON_ELEMENTAL_BOOST_MULTIPLIER_DISABLED = 1.00;
static constexpr double k_NON_ELEMENTAL_BOOST_MULTIPLIER_ACTIVE   = 1.05;

// Peak Performance                                             level: 0, 1,  2,  3
static const std::array<unsigned int, 4> peak_performance_added_raw = {0, 5, 10, 20};

// Resentment                                             level: 0, 1,  2,  3,  4,  5
static const std::array<unsigned int, 6> resentment_added_raw = {0, 5, 10, 15, 20, 25};

// Weakness Exploit                                     level: 0,  1,  2,  3
static constexpr std::array<int, 4> weakness_exploit_s1_aff = {0, 10, 15, 30};
static constexpr std::array<int, 4> weakness_exploit_s2_aff = {0, 15, 30, 50};


static double calculate_non_elemental_boost_multiplier(const Database::Database& db,
                                                       const SkillMap& skills,
                                                       const SkillSpec& skills_spec,
                                                       const Database::Weapon& weapon) {
    // TODO: Oh my god this if-statement is so messy in order to account for element acceleration. Please fix.
    if (weapon.is_raw
            && skills.binary_skill_is_lvl1(db.non_elemental_boost_ptr)
            && (skills.get_lvl(db.free_elem_ammo_up_ptr) == 0)
            &&
                (
                    (
                        (!skills_spec.get_state_for_binary_skill(db.element_acceleration_ptr))
                        && (!skills_spec.get_state_for_binary_skill(db.true_element_acceleration_ptr))
                    )
                    ||
                    (
                        (!skills.binary_skill_is_lvl1(db.element_acceleration_ptr))
                        && (!skills.binary_skill_is_lvl1(db.true_element_acceleration_ptr))
                    )
                )
            ) {
        return k_NON_ELEMENTAL_BOOST_MULTIPLIER_ACTIVE;
    } else {
        return k_NON_ELEMENTAL_BOOST_MULTIPLIER_DISABLED;
    }
}


static double calculate_raw_crit_dmg_multiplier(const Database::Database& db, const SkillMap& skills) {
    switch (skills.get_lvl(db.critical_boost_ptr)) {
        case 0: return k_RAW_CRIT_DMG_MULTIPLIER_CB0;
        case 1: return k_RAW_CRIT_DMG_MULTIPLIER_CB1;
        case 2: return k_RAW_CRIT_DMG_MULTIPLIER_CB2;
        default:
            assert(skills.get_lvl(db.critical_boost_ptr) == 3);
            return k_RAW_CRIT_DMG_MULTIPLIER_CB3;
    }
}


static double calculate_raw_sharpness_modifier(const Database::Database& db,
                                               const SkillMap& skills,
                                               const Database::Weapon& weapon,
                                               const Database::SharpnessGauge& maximum_sharpness) {
    if (weapon.is_constant_sharpness) {
        return maximum_sharpness.get_raw_sharpness_modifier();
    } else {
        const unsigned int handicraft_lvl = skills.get_lvl(db.handicraft_ptr);
        return maximum_sharpness.get_raw_sharpness_modifier(handicraft_lvl);
    }
};


SkillContribution::SkillContribution(const Database::Database&       db,
                                     const SkillMap&                 skills,
                                     const SkillSpec&                skills_spec,
                                     const Database::Weapon&         weapon,
                                     const Database::SharpnessGauge& maximum_sharpness) noexcept
    : added_raw               (0)
    , added_aff               (0)
    , neb_multiplier          (calculate_non_elemental_boost_multiplier(db, skills, skills_spec, weapon))
    , raw_crit_dmg_multiplier (calculate_raw_crit_dmg_multiplier(db, skills))
    , raw_sharpness_modifier  (calculate_raw_sharpness_modifier(db, skills, weapon, maximum_sharpness))
{
    // We calculate the remaining fields.

    if (skills_spec.get_state_for_binary_skill(db.affinity_sliding_ptr)
            && skills.binary_skill_is_lvl1(db.affinity_sliding_ptr)) {
        this->added_aff += k_AFFINITY_SLIDING_AFF;
    }

    if (skills_spec.get_state_for_binary_skill(db.agitator_ptr)) {
        const unsigned int agitator_lvl = skills.get_lvl(db.agitator_ptr, db.agitator_secret_ptr);
        this->added_raw += agitator_added_raw[agitator_lvl];
        this->added_aff += agitator_added_aff[agitator_lvl];
    }

    const unsigned int attack_boost_lvl = skills.get_lvl(db.attack_boost_ptr);
    this->added_raw += attack_boost_added_raw[attack_boost_lvl];
    this->added_aff += attack_boost_added_aff[attack_boost_lvl];

    if (skills_spec.get_state_for_binary_skill(db.coalescence_ptr)) {
        const unsigned int coalescence_lvl = skills.get_lvl(db.coalescence_ptr);
        this->added_raw += coalescence_added_raw[coalescence_lvl];
    }

    if (skills_spec.get_state_for_binary_skill(db.critical_draw_ptr)) {
        const unsigned int critical_draw_lvl = skills.get_lvl(db.critical_draw_ptr);
        this->added_aff += critical_draw_added_aff[critical_draw_lvl];
    }

    const unsigned int critical_eye_lvl = skills.get_lvl(db.critical_eye_ptr);
    this->added_aff += critical_eye_added_aff[critical_eye_lvl];

    if (skills_spec.get_state_for_binary_skill(db.dragonvein_awakening_ptr)
            || skills_spec.get_state_for_binary_skill(db.true_dragonvein_awakening_ptr)) {
        if (skills.binary_skill_is_lvl1(db.true_dragonvein_awakening_ptr)) {
            this->added_aff += k_TRUE_DRAGONVEIN_AWAKENING_AFF;
        } else if (skills.binary_skill_is_lvl1(db.dragonvein_awakening_ptr)) {
            this->added_aff += k_DRAGONVEIN_AWAKENING_AFF;
        }
    }

    if (skills_spec.get_state_for_binary_skill(db.latent_power_ptr)) {
        const unsigned int latent_power_lvl = skills.get_lvl(db.latent_power_ptr, db.latent_power_secret_ptr);
        this->added_aff += latent_power_added_aff[latent_power_lvl];
    }

    if (skills_spec.get_state_for_binary_skill(db.maximum_might_ptr)) {
        const unsigned int maximum_might_lvl = skills.get_lvl(db.maximum_might_ptr, db.maximum_might_secret_ptr);
        this->added_aff += maximum_might_added_aff[maximum_might_lvl];
    }

    if (skills_spec.get_state_for_binary_skill(db.peak_performance_ptr)) {
        const unsigned int peak_performance_lvl = skills.get_lvl(db.peak_performance_ptr);
        this->added_raw += peak_performance_added_raw[peak_performance_lvl];
    }

    if (skills_spec.get_state_for_binary_skill(db.resentment_ptr)) {
        const unsigned int resentment_lvl = skills.get_lvl(db.resentment_ptr);
        this->added_raw += resentment_added_raw[resentment_lvl];
    }

    const unsigned int wex_state = skills_spec.get_state(db.weakness_exploit_ptr);
    assert(db.weakness_exploit_ptr->states == 3);
    assert(wex_state <= 2);
    if (wex_state == 1) {
        const unsigned int wex_lvl = skills.get_lvl(db.weakness_exploit_ptr);
        this->added_aff += weakness_exploit_s1_aff[wex_lvl];
    } else if (wex_state == 2) {
        const unsigned int wex_lvl = skills.get_lvl(db.weakness_exploit_ptr); // Redundant?
        this->added_aff += weakness_exploit_s2_aff[wex_lvl];
    }
}


} // namespace

