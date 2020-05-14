/*
 * File: search_jsonparse.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <fstream>
#include <iostream>

#include "mhwi_build_search.h"
#include "utils/utils.h"

#include "../dependencies/json-3-7-3/json.hpp"


namespace MHWIBuildSearch
{


static SearchParameters read_json_obj(const Database& db, const nlohmann::json& j) {

    if (!j.is_object()) {
        throw std::runtime_error("Expected JSON object.");
    }

    const bool allow_low_rank        = j["allow_low_rank"];
    const bool allow_high_rank       = j["allow_high_rank"];
    const bool allow_master_rank     = j["allow_master_rank"];
    const bool health_regen_required = j["health_regen_required"];

    WeaponClass weapon_class = upper_snake_case_to_weaponclass(j["selected_weapon_class"]);

    std::unordered_map<const Skill*, unsigned int> min_levels;
    std::unordered_map<const Skill*, unsigned int> states;
    for (auto& e : j["selected_skills"].items()) {
        min_levels.insert({db.skills.skill_at(e.key()), e.value()});
    }
    for (auto& e : j["forced_skill_states"].items()) {
        states.insert({db.skills.skill_at(e.key()), e.value()});
    }

    SkillSpec skill_spec (std::move(min_levels),
                          std::move(states));

    return {allow_low_rank,
            allow_high_rank,
            allow_master_rank,
            health_regen_required,
    
            weapon_class,
            std::move(skill_spec)};
}


SearchParameters read_file(const Database& db, const std::string& filepath) {
    nlohmann::json j;

    {
        std::ifstream f(filepath); // open file
        f >> j;
    } // close file

    return read_json_obj(db, j);
}


} // namespace

