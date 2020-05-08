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


double calculate_non_elemental_boost_multiplier(const Database::Database&,
                                                const SkillMap&,
                                                const Database::Weapon&);

double calculate_raw_crit_dmg_multiplier(const Database::Database&,
                                         const SkillMap&);

double calculate_raw_sharpness_modifier(const Database::Database&,
                                        const SkillMap&,
                                        const Database::SharpnessGauge&);


} // namespace

#endif // SKILL_CONTRIBUTIONS_H

