/*
 * File: search_jsonparse.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <fstream>
#include <iostream>

#include "mhwi_build_search.h"
#include "database/database_miscbuffs.h"
#include "database/database_skills.h"
#include "utils/utils.h"

#include "../dependencies/json-3-7-3/json.hpp"


namespace MHWIBuildSearch
{


static SearchParameters read_json_obj(const nlohmann::json& j) {

    if (!j.is_object()) {
        throw std::runtime_error("Expected JSON object.");
    }

    const bool allow_low_rank        = j["allow_low_rank"];
    const bool allow_high_rank       = j["allow_high_rank"];
    const bool allow_master_rank     = j["allow_master_rank"];
    const bool health_regen_required = j["health_regen_required"];

    WeaponClass weapon_class = upper_snake_case_to_weaponclass(j["selected_weapon_class"]);

    std::unordered_map<const Skill*, unsigned int> min_levels = [&](){
        std::unordered_map<const Skill*, unsigned int> ret;
        for (auto& e : j["selected_skills"].items()) {
            ret.insert({SkillsDatabase::get_skill(e.key()), e.value()});
        }
        return ret;
    }();

    std::unordered_map<const Skill*, unsigned int> states = [&](){
        std::unordered_map<const Skill*, unsigned int> ret;
        for (auto& e : j["forced_skill_states"].items()) {
            ret.insert({SkillsDatabase::get_skill(e.key()), e.value()});
        }
        return ret;
    }();

    std::unordered_set<const Skill*> force_remove_skill = [&](){
        std::unordered_set<const Skill*> ret;
        nlohmann::json j2 = j["force_remove_skills"];
        if (!j2.is_array()) {
            throw std::runtime_error("'force_remove_skills' must be an array of strings.");
        }
        std::vector<std::string> force_remove_skill_ids = j2;
        for (const std::string& skill_id : force_remove_skill_ids) {
            ret.emplace(SkillsDatabase::get_skill(skill_id));
        }
        return ret;
    }();

    SkillSpec skill_spec (std::move(min_levels),
                          std::move(states),
                          std::move(force_remove_skill) );
    // TODO: Just throw an exception from within the constructor instead.
    if (!skill_spec.data_is_valid()) {
        throw std::runtime_error("Invalid query.");
    }

    // Now, we determine miscellaneous buffs here.
    std::unordered_set<const MiscBuff*> miscbuffs;
    {
        nlohmann::json j2 = j["misc_buffs"];
        if (!j2.is_array()) {
            throw std::runtime_error("'misc_buffs' must be an array of strings.");
        }
        std::vector<std::string> miscbuff_ids = j2;
        for (const std::string& miscbuff_id : miscbuff_ids) {
            const MiscBuff& miscbuff = MiscBuffsDatabase::get_miscbuff(miscbuff_id);
            miscbuffs.emplace(&miscbuff);
        }
    }

    const unsigned int model_raw_mv = j["model_raw_mv"];
    const unsigned int model_raw_hzv = j["model_raw_hzv"];

    return {allow_low_rank,
            allow_high_rank,
            allow_master_rank,
            health_regen_required,
    
            weapon_class,
            std::move(skill_spec),
            MiscBuffsEquips(std::move(miscbuffs)),

            {
                model_raw_mv,
                model_raw_hzv,
            }
            };
}


SearchParameters read_file(const std::string& filepath) {
    nlohmann::json j;

    {
        std::ifstream f(filepath); // open file
        f >> j;
    } // close file

    return read_json_obj(j);
}


} // namespace

