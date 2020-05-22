/*
 * File: database_charms.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <fstream>

#include "../../../dependencies/json-3-7-3/json.hpp"

#include "../database.h"
#include "../../utils/utils.h"
#include "../../utils/utils_strings.h"


namespace MHWIBuildSearch {


const CharmsDatabase CharmsDatabase::read_db_file(const std::string& filename, const SkillsDatabase& skills_db) {
    CharmsDatabase new_db;

    nlohmann::json j;

    {
        std::ifstream f(filename); // open file
        f >> j;
    } // close file

    assert(j.is_object());

    std::unordered_set<std::string> charm_name_check;

    // TODO: I don't like how I don't know what the type of e is.
    for (auto& e : j["charms"].items()) {
        nlohmann::json jj(e.value());

        std::string charm_id = e.key();
        if (!Utils::is_upper_snake_case(charm_id)) {
            throw std::runtime_error("Charm IDs in the charms database must be in UPPER_SNAKE_CASE.");
        }
        if (Utils::map_has_key(new_db.charms_map, charm_id)) { // UNIQUENESS CHECK
            throw std::runtime_error("Charm IDs must be unique.");
        }

        std::string name = jj["name"];
        if (!Utils::has_ascii_letters(name)) {
            throw std::runtime_error("Charm names must have at least one ASCII letter (A-Z or a-z).");
        }
        if (Utils::map_has_key(charm_name_check, name)) { // UNIQUENESS CHECK
            throw std::runtime_error("Charm names must be unique.");
        }
        charm_name_check.insert(name);

        const unsigned int max_charm_lvl = jj["max_level"];

        std::vector<const Skill*> skills;
        if (!jj["skills"].is_array()) {
            throw std::runtime_error("Charm skills must be arrays of skill IDs.");
        }
        for (auto& e : jj["skills"].items()) {
            const std::string skill_id = e.value();
            const Skill* const skill = skills_db.skill_at(skill_id);

            if (max_charm_lvl > skill->secret_limit) {
                throw std::runtime_error("Charm levels must not exceed their skill maximum levels. "
                                         "Offending charm: '" + charm_id + "'.");
            }

            skills.push_back(skill);
        }

        std::string charm_id_copy = charm_id; // Must use this for move to work.
        new_db.charms_map.insert({charm_id, std::make_shared<Charm>(std::move(charm_id_copy),
                                                                    std::move(name),
                                                                    std::move(max_charm_lvl),
                                                                    std::move(skills)) });
    }

    return new_db;
}


const Charm* CharmsDatabase::at(const std::string& charm_id) const {
    assert(Utils::is_upper_snake_case(charm_id));
    try {
        return this->charms_map.at(charm_id).get();
    } catch (const std::out_of_range&) {
        throw std::out_of_range("Charm ID '" + charm_id + "' not found in the database.");
    }
}


std::vector<const Charm*> CharmsDatabase::get_all() const {
    std::vector<const Charm*> ret;
    for (const auto& e : this->charms_map) {
        ret.emplace_back(e.second.get());
    }
    return ret;
}


CharmsDatabase::CharmsDatabase() noexcept = default;


} // namespace

