/*
 * File: database_skills.cpp
 * Author: <contact@simshadows.com>
 */

#include <iostream>
#include <fstream>
#include <unordered_set>

#include "../../dependencies/json-3-7-3/json.hpp"

#include "database.h"


namespace Database {


constexpr unsigned int k_MINIMUM_SKILL_STATES = 2;


Skill::Skill(std::string new_id,
             std::string new_name,
             unsigned int new_normal_limit,
             unsigned int new_secret_limit,
             unsigned int new_states) noexcept
    : id           (new_id)
    , name         (new_name)
    , normal_limit (new_normal_limit)
    , secret_limit (new_secret_limit)
    , states       (new_states)
{
}


// static
const SkillsDatabase SkillsDatabase::read_db_file(const std::string& filename) {
    SkillsDatabase new_db;

    nlohmann::json j;

    {
        std::ifstream f(filename); // open file
        f >> j;
    } // close file

    assert(j.is_object());

    /*
     * Stage 1: Read skills.
     */

    std::unordered_set<std::string> skill_name_check;

    // TODO: I don't like how I don't know what the type of e is.
    for (auto& e : j["skills"].items()) {
        nlohmann::json jj(e.value());

        std::string skill_id = e.key();
        if (!Utils::is_upper_snake_case(skill_id)) {
            throw std::runtime_error("Skill IDs in the skills database must be in UPPER_SNAKE_CASE.");
        }
        if (Utils::map_has_key(new_db.skills_map, skill_id)) { // UNIQUENESS CHECK
            throw std::runtime_error("Skill IDs must be unique.");
        }

        std::string name = jj["name"];
        if (!Utils::has_ascii_letters(name)) {
            throw std::runtime_error("Skill names must have at least one ASCII letter (A-Z or a-z).");
        }
        if (Utils::map_has_key(skill_name_check, name)) { // UNIQUENESS CHECK
            throw std::runtime_error("Skill names must be unique.");
        }
        skill_name_check.insert(name);

        unsigned int normal_limit = jj["limit"];
        unsigned int secret_limit;
        if (jj["secret_limit"].is_null()) {
            secret_limit = normal_limit;
        } else {
            secret_limit = jj["secret_limit"];
        }
        if (normal_limit == 0) {
            throw std::runtime_error("All skills must have at least one possible level.");
        } else if (secret_limit < normal_limit) {
            throw std::runtime_error("Skill secret level limit must not be lower than the normal limit.");
        }

        unsigned int states;
        if (jj["states"].is_null()) {
            states = k_MINIMUM_SKILL_STATES;
        } else {
            states = jj["states"];
        }
        if (states < 2) {
            throw std::runtime_error("All skills must have at least two possible states.");
        }

        new_db.skills_map.insert({skill_id, std::make_shared<Skill>(skill_id,
                                               
                                                                    name,
                                                                    normal_limit,
                                                                    secret_limit,

                                                                    states) });

    }

    /*
     * Stage 2: Read set bonuses.
     */

    std::unordered_set<std::string> sb_name_check;

    // TODO: I don't like how I don't know what the type of e is.
    for (auto& e : j["set_bonuses"].items()) {
        nlohmann::json jj(e.value());

        std::string sb_id = e.key();
        if (!Utils::is_upper_snake_case(sb_id)) {
            throw std::runtime_error("Set bonus IDs in the skills database must be in UPPER_SNAKE_CASE.");
        }
        if (Utils::map_has_key(new_db.set_bonuses_map, sb_id)) { // UNIQUENESS CHECK
            throw std::runtime_error("Set bonus IDs must be unique.");
        }

        std::string name = jj["name"];
        if (!Utils::has_ascii_letters(name)) {
            throw std::runtime_error("Set bonus names must have at least one ASCII letter (A-Z or a-z).");
        }
        if (Utils::map_has_key(sb_name_check, name)) { // UNIQUENESS CHECK
            throw std::runtime_error("Set bonus names must be unique.");
        }
        sb_name_check.insert(name);

        std::vector<std::tuple<unsigned int, const Skill*>> stages;
        for (auto& stage : jj["stages"]) {
            nlohmann::json jjj(stage);

            const unsigned int parts = jjj["parts"];
            const Skill* sb_skill = new_db.skill_at(jjj["skill"]);

            stages.emplace_back(parts, sb_skill);
        }

        new_db.set_bonuses_map.insert({sb_id, std::make_shared<SetBonus>(std::move(sb_id),
                                                                         std::move(name),
                                                                         std::move(stages)) });
    }

    return new_db;
}


const Skill* SkillsDatabase::skill_at(const std::string& skill_id) const {
    assert(Utils::is_upper_snake_case(skill_id));
    //Skill* p = this->skills_map.at(skill_id).get(); // TODO: Delete this line
    try {
        return this->skills_map.at(skill_id).get();
    } catch (std::out_of_range) {
        throw std::out_of_range("Skill ID '" + skill_id + "' not found in the database.");
    }
}


SkillsDatabase::SkillsDatabase() noexcept = default;


} // namespace
