/*
 * File: skill_contributions.h
 * Author: <contact@simshadows.com>
 */

#ifndef SKILL_CONTRIBUTIONS_H
#define SKILL_CONTRIBUTIONS_H

#include "containers.h"
#include "database/database.h"

namespace MHWIBuildSearch
{


struct SkillContribution {
    unsigned int added_raw;
    int          added_aff;
    double       neb_multiplier;
    double       raw_crit_dmg_multiplier;
    double       raw_sharpness_modifier;

    SkillContribution(const Database::Database&,
                      const SkillMap&,
                      const SkillSpec&,
                      const Database::Weapon&,
                      const Database::SharpnessGauge&) noexcept;
};


} // namespace

#endif // SKILL_CONTRIBUTIONS_H

