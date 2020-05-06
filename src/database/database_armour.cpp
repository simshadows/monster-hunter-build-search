/*
 * File: database_armour.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <fstream>

#include "../../dependencies/json-3-7-3/json.hpp"

#include "database.h"


namespace Database {


static constexpr std::size_t k_NUM_POSSIBLE_SLOTS = 5;
static constexpr std::size_t k_HEAD_INDEX  = 0;
static constexpr std::size_t k_CHEST_INDEX = 1;
static constexpr std::size_t k_ARMS_INDEX  = 2;
static constexpr std::size_t k_WAIST_INDEX = 3;
static constexpr std::size_t k_LEGS_INDEX  = 4;

// Postfixes in an armour set naming scheme that are intended to be unused must be empty strings.
// This will be used for validation, in case an armour set accidentally attempts to use an
// invalidated postfix.
static constexpr char k_INVALIDATED_POSTFIX[] = "";


const ArmourDatabase ArmourDatabase::read_db_file(const std::string& filename, const SkillsDatabase& skills_db) {
    (void)skills_db;
    ArmourDatabase new_db;

    nlohmann::json j;

    {
        std::ifstream f(filename); // open file
        f >> j;
    } // close file

    assert(j.is_object());

    std::unordered_map<std::string, std::vector<std::string>> naming_schemes;

    // TODO: I don't like how I don't know what the type of e is.
    for (auto& e : j["naming_schemes"].items()) {
        nlohmann::json jj(e.value());

        std::string scheme_id = e.key();
        if (!Utils::is_upper_snake_case(scheme_id)) {
            throw std::runtime_error("Armour naming scheme IDs in the armour database must be in UPPER_SNAKE_CASE. "
                                     "Offending ID: '" + scheme_id + "'.");
        }
        if (Utils::map_has_key(naming_schemes, scheme_id)) { // UNIQUENESS CHECK
            throw std::runtime_error("Armour naming scheme IDs must be unique.");
        }

        std::vector<std::string> postfixes = jj;
        if (postfixes.size() != k_NUM_POSSIBLE_SLOTS) {
            throw std::runtime_error("There must be exactly " + std::to_string(k_NUM_POSSIBLE_SLOTS)
                                     + " entries in a naming scheme. Offending ID: '" + scheme_id + "'.");
        }

        // TODO: Is this a guaranteed move?
        naming_schemes.insert({std::move(scheme_id), std::move(postfixes)});
    }

    return new_db;
}


ArmourDatabase::ArmourDatabase() noexcept = default;


} // namespace

