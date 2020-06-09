/*
 * File: mhwi_build_search.h
 * Author: <contact@simshadows.com>
 */

#include "core/core.h"
#include "database/database.h"
#include "support/support.h"

namespace MHWIBuildSearch
{


/****************************************************************************************
 * search_jsonparse: JSON Search Parameter Parsing
 ***************************************************************************************/


struct SearchParameters {
    bool allow_low_rank;
    bool allow_high_rank;
    bool allow_master_rank;

    WeaponClass weapon_class;
    std::unordered_set<EleStatType> allowed_weapon_elestat_types;
    bool health_regen_required;

    DamageModel damage_model;

    SkillSpec skill_spec;
    MiscBuffsEquips misc_buffs;
};


SearchParameters read_file(const std::string& filepath);


/****************************************************************************************
 * search
 ***************************************************************************************/


void search_cmd(const std::string& search_parameters_path);


} // namespace

