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


SkillMap::SkillMap(const ArmourPiece& armour_piece) noexcept
{
    for (const auto& e : armour_piece.skills) {
        this->data.insert(e);
    }
}


void SkillMap::merge_in(const DecoEquips& decos) {
    for (const Decoration* const& deco : decos) {
        for (const auto& e : deco->skills) {
            this->increment(e.first, e.second);
        }
    }
}


void SkillMap::add_set_bonuses(const SetBonusMap& set_bonuses) {
    for (const auto& e : set_bonuses) {
        const SetBonus * const set_bonus = e.first;
        const unsigned int present_pieces = e.second;
        for (const auto& ee : set_bonus->stages) {
            const unsigned int required_pieces = ee.first;
            const Skill * const skill = ee.second;
            if (present_pieces >= required_pieces) this->increment(skill, 1);
        }
    }
}


void SkillMap::add_skills_filtered(const ArmourPiece& piece, const SkillSpec& skill_spec) {
    for (const std::pair<const Skill*, unsigned int>& e : piece.skills) {
        if (skill_spec.is_in_subset(e.first)) {
            this->increment(e.first, e.second);
        }
    }
    assert(this->only_contains_skills_in_spec(skill_spec));
}


void SkillMap::add_skills_filtered(const Charm& charm, const unsigned int charm_lvl, const SkillSpec& skill_spec) {
    assert(charm_lvl);
    for (const Skill * const skill : charm.skills) {
        if (skill_spec.is_in_subset(skill)) {
            this->increment(skill, charm_lvl);
        }
    }
    assert(this->only_contains_skills_in_spec(skill_spec));
}


void SkillMap::add_skills_filtered(const std::vector<const Decoration*>& decos, const SkillSpec& skill_spec) {
    for (const Decoration * const deco : decos) {
        for (const auto& e : deco->skills) {
            if (skill_spec.is_in_subset(e.first)) {
                this->increment(e.first, e.second);
            }
        }
    }
    assert(this->only_contains_skills_in_spec(skill_spec));
}


unsigned int SkillMap::get_non_secret(const Skill * const skill,
                                      const Skill * const associated_secret) const {
    // We expect secret skills to only have one level.
    assert(associated_secret->secret_limit == 1);

    if (Utils::map_has_key(this->data, skill)) {
        const unsigned int level = this->data.at(skill);
        const unsigned int normal_limit = skill->normal_limit;
        if (Utils::map_has_key(this->data, associated_secret) || (level <= normal_limit)) {
            return this->data.at(skill);
        } else {
            return normal_limit;
        }
    } else {
        return 0;
    }
}


bool SkillMap::is_at_least_lvl1(const Skill* skill) const {
    const bool ret = (Utils::map_has_key(this->data, skill));
    assert(ret == (this->get(skill) > 0)); // TODO: Use "!=" for xor? Or "==" for xnor?
    return ret;
}


bool SkillMap::binary_skill_is_lvl1(const Skill* skill) const {
    assert(skill->normal_limit == 1); // Must be a binary skill
    assert(skill->secret_limit == 1); // Must not have a secret skill (for now)
    const bool ret = Utils::map_has_key(this->data, skill);
    assert((!ret) || (this->data.at(skill) == 1)); // If skill is present, its level is 1.
    return ret;
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

