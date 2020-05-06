/*
 * File: containers.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include "containers.h"


namespace MHWIBuildSearch
{


SkillMap::SkillMap() noexcept = default;


void SkillMap::set_lvl(const Database::Skill* skill, unsigned int level) {
    assert(level != 0); // For now, I won't intend to ever use this for removing skills.
    if (level > skill->secret_limit) {
        level = skill->secret_limit; // Clips the level
    }
    this->data[skill] = level;
}


void SkillMap::increment_lvl(const Database::Skill* skill, const unsigned int level_to_add) {
    assert(level_to_add != 0); // For now, I won't intend to ever use this for zero increments.
    if (this->data.count(skill) == 1) {
        unsigned int new_level = this->data.at(skill) + level_to_add;
        if (new_level > skill->secret_limit) {
            new_level = skill->secret_limit; // Clips the level
        }
        this->data[skill] = new_level;
    } else {
        assert(this->data.count(skill) == 0); // count shouldn't ever have any other value.
        this->data[skill] = level_to_add;
    }
}


unsigned int SkillMap::get_lvl(const Database::Skill* skill) const {
    if (this->data.count(skill) == 1) {
        return this->data.at(skill);
    } else {
        assert(this->data.count(skill) == 0); // count shouldn't ever have any other value.
        return 0;
    }
}


//unsigned int SkillMap::get_lvl(const std::string& skill_id) const {
//    for (const auto& e : this->data) {
//        if (e.first->id == skill_id) return e.second;
//    }
//    throw std::runtime_error("Skill '" + skill_id + "' was not found in the SkillMap.");
//}


std::string SkillMap::get_humanreadable() const {
    std::string ret;
    bool is_first = true;
    for (const auto& e : this->data) {
        if (!is_first) ret += "\n";
        ret += e.first->name + " = " + std::to_string(e.second);
        is_first = false;
    }
    return ret;
}


} // namespace

