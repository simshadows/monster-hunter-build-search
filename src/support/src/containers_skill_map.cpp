/*
 * File: containers_skill_map.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <algorithm>

#include "../support.h"
#include "../../utils/utils.h"


namespace MHWIBuildSearch
{


SkillMap::SkillMap() noexcept = default;


SkillMap::SkillMap(const ArmourPiece& armour_piece) noexcept
    : data {}
{
    for (const auto& e : armour_piece.skills) {
        this->data.insert(e);
    }
}


void SkillMap::set_lvl(const Skill* skill, unsigned int level) {
    assert(level != 0); // For now, I won't intend to ever use this for removing skills.
    if (level > skill->secret_limit) {
        level = skill->secret_limit; // Clips the level
    }
    this->data[skill] = level;
}


void SkillMap::increment_lvl(const Skill* skill, const unsigned int lvl_to_add) {
    assert(lvl_to_add != 0); // For now, I won't intend to ever use this for zero increments.
    if (this->data.count(skill) == 1) {
        unsigned int new_level = this->data.at(skill) + lvl_to_add;
        if (new_level > skill->secret_limit) {
            new_level = skill->secret_limit; // Clips the level
        }
        this->data[skill] = new_level;
    } else {
        assert(this->data.count(skill) == 0); // count shouldn't ever have any other value.
        this->data[skill] = lvl_to_add;
    }
}


void SkillMap::decrement_lvl(const Skill* skill, unsigned int lvl_to_remove) {
    assert(lvl_to_remove != 0); // For now, I won't intend to ever use this for zero decrements.
    assert(this->data.count(skill) == 1);
    unsigned int curr_lvl = this->data.at(skill);
    if (curr_lvl == lvl_to_remove) {
        this->data.erase(skill);
    } else {
        assert(curr_lvl > lvl_to_remove); // For now, we don't accept invalid values of lvl_to_remove.
        this->data.at(skill) = curr_lvl - lvl_to_remove;
    }
}


void SkillMap::add_skills(const ArmourPiece& piece) {
    for (const std::pair<const Skill*, unsigned int>& e : piece.skills) {
        this->increment_lvl(e.first, e.second);
    }
}


void SkillMap::add_skills(const DecoEquips& decos) {
    for (const Decoration* const& deco : decos) {
        for (const auto& e : deco->skills) {
            this->increment_lvl(e.first, e.second);
        }
    }
}




void SkillMap::add_set_bonuses(const std::unordered_map<const SetBonus*, unsigned int>& set_bonuses) {
    for (const auto& e : set_bonuses) {
        const SetBonus * const set_bonus = e.first;
        const unsigned int present_pieces = e.second;
        for (const auto& ee : set_bonus->stages) {
            const unsigned int required_pieces = ee.first;
            const Skill * const skill = ee.second;
            if (present_pieces >= required_pieces) this->increment_lvl(skill, 1);
        }
    }
}


void SkillMap::remove_skill(const Skill* skill) {
    assert(this->get_lvl(skill));
    this->data.erase(skill);
}


void SkillMap::add_skills_filtered(const ArmourPiece& piece, const SkillSpec& skill_spec) {
    for (const std::pair<const Skill*, unsigned int>& e : piece.skills) {
        if (skill_spec.is_in_subset(e.first)) {
            this->increment_lvl(e.first, e.second);
        }
    }
    assert(this->only_contains_skills_in_spec(skill_spec));
}


void SkillMap::add_skills_filtered(const Charm& charm, const unsigned int charm_lvl, const SkillSpec& skill_spec) {
    assert(charm_lvl);
    for (const Skill * const skill : charm.skills) {
        if (skill_spec.is_in_subset(skill)) {
            this->increment_lvl(skill, charm_lvl);
        }
    }
    assert(this->only_contains_skills_in_spec(skill_spec));
}


void SkillMap::add_skills_filtered(const std::vector<const Decoration*>& decos, const SkillSpec& skill_spec) {
    for (const Decoration * const deco : decos) {
        for (const auto& e : deco->skills) {
            if (skill_spec.is_in_subset(e.first)) {
                this->increment_lvl(e.first, e.second);
            }
        }
    }
    assert(this->only_contains_skills_in_spec(skill_spec));
}


void SkillMap::merge_in(const SkillMap& obj) {
    for (const auto& e : obj.data) {
        this->increment_lvl(e.first, e.second);
    }
}


unsigned int SkillMap::get_lvl(const Skill * const skill) const {
    assert(this->data.count(skill) <= 1); // count must return either 1 or 0.
    return this->data.count(skill) ? this->data.at(skill) : 0;
}


//unsigned int SkillMap::get_lvl_no_secret(const Skill * const skill) const {
//    assert(this->data.count(skill) <= 1); // count must return either 1 or 0.
//    const unsigned int normal_limit = skill->normal_limit;
//    if (Utils::map_has_key(this->data, skill)) {
//        const unsigned int level = this->data.at(skill);
//        return (level > normal_limit) ? normal_limit : level;
//    } else {
//        return 0;
//    }
//}


unsigned int SkillMap::get_lvl(const Skill * const skill,
                               const Skill * const associated_secret) const {
    // count must return either 1 or 0.
    assert(this->data.count(skill) <= 1);
    assert(this->data.count(associated_secret) <= 1);
    // We expect secret skills to only have one level.
    assert(associated_secret->secret_limit == 1);

    if (this->data.count(skill)) {
        const unsigned int level = this->data.at(skill);
        const unsigned int normal_limit = skill->normal_limit;
        return (this->data.count(associated_secret) || (level <= normal_limit)) ? this->data.at(skill) : normal_limit;
    } else {
        return 0;
    }
}


bool SkillMap::is_at_least_lvl1(const Skill* skill) const {
    const bool ret = (Utils::map_has_key(this->data, skill));
    assert(ret == (this->get_lvl(skill) > 0)); // TODO: Use "!=" for xor? Or "==" for xnor?
    return ret;
}


bool SkillMap::binary_skill_is_lvl1(const Skill* skill) const {
    assert(skill->normal_limit == 1); // Must be a binary skill
    assert(skill->secret_limit == 1); // Must not have a secret skill (for now)
    const bool ret = Utils::map_has_key(this->data, skill);
    assert((!ret) || (this->data.at(skill) == 1)); // If skill is present, its level is 1.
    return ret;
}


//unsigned int SkillMap::get_lvl(const std::string& skill_id) const {
//    for (const auto& e : this->data) {
//        if (e.first->id == skill_id) return e.second;
//    }
//    throw std::runtime_error("Skill '" + skill_id + "' was not found in the SkillMap.");
//}


std::size_t SkillMap::calculate_hash() const noexcept {
    std::size_t ret = 0;
    for (const auto& e : this->data) {
        const std::size_t v = ((std::size_t) e.first) * e.second;
        ret += v;
    }
    return ret;
}


SkillMap::IteratorType SkillMap::begin() const {
    return this->data.begin();
}


SkillMap::IteratorType SkillMap::end() const {
    return this->data.end();
}


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


bool SkillMap::only_contains_skills_in_spec(const SkillSpec& skill_spec) const noexcept {
    for (const auto& e : this->data) {
        assert(e.second);
        if (!skill_spec.is_in_subset(e.first)) return false;
    }
    return true;
}


} // namespace

