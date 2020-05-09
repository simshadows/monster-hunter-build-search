/*
 * File: skill_contributions.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include "skill_contributions.h"

namespace MHWIBuildSearch
{


static constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB0 = 1.25; // Critical Boost 0
static constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB1 = 1.30; // Critical Boost 1
static constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB2 = 1.35; // Critical Boost 2
static constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB3 = 1.40; // Critical Boost 3

static constexpr double k_NON_ELEMENTAL_BOOST_MULTIPLIER_DISABLED = 1.00;
static constexpr double k_NON_ELEMENTAL_BOOST_MULTIPLIER_ACTIVE = 1.05;

                                                      // level 0, 1, 2,  3,  4,  5,  6,  7
static const std::array<unsigned int, 8> agitator_added_raw = {0, 4, 8, 12, 16, 20, 24, 28};
static const std::array<int         , 8> agitator_added_aff = {0, 5, 5, 7,  7,  10, 15, 20};


static double calculate_non_elemental_boost_multiplier(const Database::Database& db,
                                                       const SkillMap& skills,
                                                       const Database::Weapon& weapon) {
    if ((!weapon.is_raw) || (skills.get_lvl(db.non_elemental_boost_ptr) == 0)) {
        return k_NON_ELEMENTAL_BOOST_MULTIPLIER_DISABLED;
    } else {
        assert(skills.get_lvl(db.non_elemental_boost_ptr) == 1);
        return k_NON_ELEMENTAL_BOOST_MULTIPLIER_ACTIVE;
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
                                     const Database::Weapon&         weapon,
                                     const Database::SharpnessGauge& maximum_sharpness) noexcept
    : added_raw               (0)
    , added_aff               (0)
    , neb_multiplier          (calculate_non_elemental_boost_multiplier(db, skills, weapon))
    , raw_crit_dmg_multiplier (calculate_raw_crit_dmg_multiplier(db, skills))
    , raw_sharpness_modifier  (calculate_raw_sharpness_modifier(db, skills, weapon, maximum_sharpness))
{
    // We calculate the remaining fields.
    unsigned int agitator_lvl = skills.get_lvl(db.agitator_ptr, db.agitator_secret_ptr);
    assert(agitator_lvl < agitator_added_raw.size());
    this->added_raw += agitator_added_raw[agitator_lvl];
    assert(agitator_lvl < agitator_added_aff.size());
    this->added_aff += agitator_added_aff[agitator_lvl];
}


} // namespace

