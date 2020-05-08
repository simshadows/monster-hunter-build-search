/*
 * File: skill_contributions.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include "containers.h"
#include "database/database.h"

namespace MHWIBuildSearch
{


constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB0 = 1.25; // Critical Boost 0
constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB1 = 1.30; // Critical Boost 1
constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB2 = 1.35; // Critical Boost 2
constexpr double k_RAW_CRIT_DMG_MULTIPLIER_CB3 = 1.40; // Critical Boost 3

static constexpr double k_NON_ELEMENTAL_BOOST_MULTIPLIER = 1.05;


double calculate_non_elemental_boost_multiplier(const Database::Database& db,
                                                const SkillMap& skills,
                                                const Database::Weapon& weapon) {
    if ((!weapon.is_raw) || (skills.get_lvl(db.non_elemental_boost_ptr) == 0)) {
        return 1.0;
    } else {
        assert(skills.get_lvl(db.non_elemental_boost_ptr) == 1);
        return k_NON_ELEMENTAL_BOOST_MULTIPLIER;
    }
}


double calculate_raw_crit_dmg_multiplier(const Database::Database& db, const SkillMap& skills) {
    switch (skills.get_lvl(db.critical_boost_ptr)) {
        case 0: return k_RAW_CRIT_DMG_MULTIPLIER_CB0;
        case 1: return k_RAW_CRIT_DMG_MULTIPLIER_CB1;
        case 2: return k_RAW_CRIT_DMG_MULTIPLIER_CB2;
        default:
            assert(skills.get_lvl(db.critical_boost_ptr) == 3);
            return k_RAW_CRIT_DMG_MULTIPLIER_CB3;
    }
}


double calculate_raw_sharpness_modifier(const Database::Database& db,
                                        const SkillMap& skills,
                                        const Database::SharpnessGauge& maximum_sharpness) {
    const unsigned int handicraft_lvl = skills.get_lvl(db.handicraft_ptr);
    return maximum_sharpness.get_raw_sharpness_modifier(handicraft_lvl);
};


} // namespace

