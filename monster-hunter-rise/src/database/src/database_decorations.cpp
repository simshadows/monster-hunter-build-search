/*
 * File: database_decorations.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <fstream>

#include "../../../dependencies/json-3-9-1/json.hpp"

#include "../database.h"
#include "../database_skills.h"
#include "../../utils/utils.h"
#include "../../utils/utils_strings.h"


namespace MHRBuildSearch {


static constexpr unsigned int k_SIMPLE_DECO_SKILL_LVL = 1;
static constexpr unsigned int k_COMPLEX_DECO_SIZE = 4;


const DecorationsDatabase DecorationsDatabase::read_db_file(const std::string& filename) {
    DecorationsDatabase new_db;

    nlohmann::json j;

    {
        std::ifstream f(filename); // open file
        f >> j;
    } // close file

    assert(j.is_object());

    /*
     * Stage 1: Simple size 1-3 decorations.
     */

    std::unordered_map<std::string, std::pair<const Skill*, std::string>> simple_deco_to_skill;

    // TODO: I don't like how I don't know what the type of e is.
    for (auto& e : j["simple_decorations"].items()) {
        nlohmann::json jj(e.value());

        std::string deco_id = e.key();
        if (!Utils::is_upper_snake_case(deco_id)) {
            throw std::runtime_error("Decoration IDs in the decorations database must be in UPPER_SNAKE_CASE.");
        }
        if (Utils::map_has_key(new_db.decorations_store, deco_id)) { // UNIQUENESS CHECK
            throw std::runtime_error("Decoration IDs must be unique.");
        }

        const unsigned int slot_size = jj["slot"];
        if ((slot_size < k_MIN_DECO_SIZE) || (slot_size > k_MAX_DECO_SIZE)) {
            throw std::runtime_error("Simple decorations must have a slot size of 1, 2, or 3.");
        }

        std::string base_name = jj["name"];
        std::string name = ((std::string) jj["name"]) + " Jewel " + std::to_string(slot_size);
        if (!Utils::has_ascii_letters(base_name)) { // SANITY CHECK
            throw std::runtime_error("Decoration names must have at least one ASCII letter (A-Z or a-z).");
        }

        std::string skill_name = jj["skill"];
        const Skill * const skill = SkillsDatabase::get_skill(skill_name);
        std::vector<std::pair<const Skill*, unsigned int>> skills = {
            {skill, k_SIMPLE_DECO_SKILL_LVL},
        };

        std::string deco_id_copy = deco_id;
        new_db.decorations_store.insert({deco_id, std::make_shared<Decoration>(std::move(deco_id_copy),
                                                                               std::move(name),
                                                                               slot_size,
                                                                               std::move(skills)) });
        simple_deco_to_skill.insert({deco_id, {skill, std::move(base_name)}});
    }

    /*
     * Stage 2: Size 4 compound decorations.
     */

    nlohmann::json j2 = j["4slot_compound_decorations_batch_definitions"];
    if (!j2.is_array()) {
        throw std::runtime_error("Value of key 4slot_compound_decorations_batch_definitions must be an array.");
    }

    // TODO: I don't like how I don't know what the type of e is.
    for (auto& e : j2.items()) {
        nlohmann::json jj(e.value());

        std::vector<std::string> left_ids = jj["left_side"];
        std::vector<std::string> right_ids = jj["right_side"];

        for (const std::string& left_id : left_ids) {
           const std::pair<const Skill*, std::string> left = simple_deco_to_skill.at(left_id);
            for (const std::string& right_id : right_ids) {
                const std::pair<const Skill*, std::string> right = simple_deco_to_skill.at(right_id);
                
                if (left.first == right.first) {
                    throw std::runtime_error("Compound decorations cannot have the same skill on either side.");
                }

                std::string deco_id = left_id + "_" + right_id + "_COMPOUND";
                std::string name = left.second + "/" + right.second + " Jewel 4";
                std::vector<std::pair<const Skill*, unsigned int>> skills = {
                    {left.first,  k_SIMPLE_DECO_SKILL_LVL},
                    {right.first, k_SIMPLE_DECO_SKILL_LVL},
                };

                std::string deco_id_copy = deco_id;
                new_db.decorations_store.insert({deco_id, std::make_shared<Decoration>(std::move(deco_id_copy),
                                                                                       std::move(name),
                                                                                       k_COMPLEX_DECO_SIZE,
                                                                                       std::move(skills)) });
            }
        }
    }

    /*
     * Stage 3: Size 4 single-skill decorations.
     */

    nlohmann::json j3 = j["4slot_single_skill_decorations"];

    // TODO: I don't like how I don't know what the type of e is.
    for (auto& e : j3.items()) {
        std::string simple_deco_id = e.key();
        std::pair<const Skill*, std::string> t = simple_deco_to_skill.at(simple_deco_id);

        std::vector<unsigned int> skill_levels = e.value();
        if (skill_levels.size() == 0) {
            throw std::runtime_error("Size-4 single-skill decorations must have at least one value.");
        }

        for (const unsigned int& skill_level : skill_levels) {
            std::string deco_id = simple_deco_id + "_" + std::to_string(skill_level) + "X";

            std::string name;
            if (skill_level == 2) {
                name = t.second + " Jewel+ 4";
            } else if (skill_level == 3) {
                name = "Hard " + t.second + " Jewel 4";
            } else {
                throw std::runtime_error("Currently unsupported size-4 single-skill decoration size.");
            }

            std::vector<std::pair<const Skill*, unsigned int>> skills = {
                {t.first, skill_level},
            };

            std::string deco_id_copy = deco_id;
            new_db.decorations_store.insert({deco_id, std::make_shared<Decoration>(std::move(deco_id_copy),
                                                                                   std::move(name),
                                                                                   k_COMPLEX_DECO_SIZE,
                                                                                   std::move(skills)) });
        }
    }

    return new_db;
}


const Decoration* DecorationsDatabase::at(const std::string& deco_id) const {
    return this->decorations_store.at(deco_id).get();
}


std::vector<const Decoration*> DecorationsDatabase::get_all() const {
    std::vector<const Decoration*> ret;

    for (const auto& e : this->decorations_store) {
        ret.emplace_back(e.second.get());
    }
    return ret;
}


std::unordered_set<const Skill*> DecorationsDatabase::all_possible_skills_from_decos() const {
    std::unordered_set<const Skill*> ret;
    for (const auto& e : this->decorations_store) {
        for (const auto& ee : e.second->skills) {
            ret.emplace(ee.first);
        }
    }
    return ret;
}


DecorationsDatabase::DecorationsDatabase() noexcept = default;


} // namespace

