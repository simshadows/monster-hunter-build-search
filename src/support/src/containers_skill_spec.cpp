/*
 * File: containers_skill_spec.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <algorithm>

#include "../../database/database_skills.h"
#include "../support.h"
#include "../../utils/utils.h"


namespace MHWIBuildSearch
{


static constexpr unsigned int k_DEACTIVATED_STATE = 0;


SkillSpec::SkillSpec(InputContainer&& new_min_levels,
                     InputContainer&& forced_states,
                     SkillSet&& new_force_remove_skills ) noexcept
    : min_levels          (std::move(new_min_levels))
    , states              (std::move(forced_states))
    , force_remove_skills (std::move(new_force_remove_skills))
    , set_bonus_cutoffs   {}
{
    for (const auto& e : this->min_levels) {
        const Skill * const skill = e.first;
        if (!Utils::map_has_key(this->states, skill)) {
            const unsigned int max_state = skill->states - 1;
            this->states.emplace(skill, max_state);
        }
    }

    // TODO: Make this a static function?
    std::unordered_map<const SetBonus*, unsigned int> setbonus_cutoffs_intermediate;
    for (const SetBonus * const set_bonus : SkillsDatabase::g_all_setbonuses) {
        for (const auto& e : set_bonus->stages) {
            if (Utils::set_has_key(this->force_remove_skills, e.second)) {
                const auto result = setbonus_cutoffs_intermediate.find(set_bonus);
                if ((result != setbonus_cutoffs_intermediate.end()) && (result->second > e.first)) {
                    result->second = e.first;
                } else {
                    setbonus_cutoffs_intermediate.emplace(std::make_pair(set_bonus, e.first));
                }
            }
        }
    }
    this->set_bonus_cutoffs.assign(setbonus_cutoffs_intermediate.begin(), setbonus_cutoffs_intermediate.end());

    assert(this->data_is_valid());
}


bool SkillSpec::is_in_subset(const Skill* skill) const {
    return Utils::map_has_key(this->min_levels, skill);
}


unsigned int SkillSpec::get_min_lvl(const Skill* skill) const {
    const auto& result = this->min_levels.find(skill);
    return (result == this->min_levels.end()) ? 0 : result->second;
}


unsigned int SkillSpec::get_state(const Skill * const skill) const {
    const auto& result = this->states.find(skill);
    return (result == this->states.end()) ? k_DEACTIVATED_STATE : result->second;
}


bool SkillSpec::get_state_for_binary_skill(const Skill * const skill) const {
    assert(skill->states == 2);
    const auto& result = this->states.find(skill);
    return result != this->states.end();
}


bool SkillSpec::skills_meet_minimum_requirements(const SkillMap& skills) const {
    for (const auto& e : this->min_levels) {
        if (skills.get(e.first) < e.second) return false;
    }
    return true;
}


bool SkillSpec::skill_must_be_removed(const Skill* skill) const {
    return Utils::set_has_key(this->force_remove_skills, skill);
}


const std::vector<std::pair<const SetBonus*, unsigned int>>& SkillSpec::get_set_bonus_cutoffs() const {
    return this->set_bonus_cutoffs;
}


std::vector<const Skill*> SkillSpec::get_skill_subset_as_vector() const {
    std::vector<const Skill*> ret;
    for (const auto& e : this->min_levels) {
        ret.emplace_back(e.first);
    }
    return ret;
}


SkillSpec::MinLevelsIterator SkillSpec::begin() const {
    return this->min_levels.begin();
}

SkillSpec::MinLevelsIterator SkillSpec::end() const {
    return this->min_levels.end();
}


std::string SkillSpec::get_humanreadable() const {
    assert(this->data_is_valid());
    std::string ret = "Skill subset:";

    if (min_levels.size() == 0) {
        ret += "\n  (no skills)";
    } else {
        for (const auto& e : this->min_levels) {
            const Skill * const skill   = e.first;
            const unsigned int  min_lvl = e.second;
            const unsigned int  state   = this->states.at(skill);
            assert(state != 0);

            // TODO: Figure out a better solution to converting char* to std::string.
            ret += "\n  " + std::string(skill->name);
            if (min_lvl > 0)       ret += " [Minimum level: " + std::to_string(min_lvl) + "]";
            if (skill->states > 2) ret += " [State: " + std::to_string(state) + "]";
        }
    }

    ret += "\n\nForce-remove skills:";
    if (force_remove_skills.size() == 0) {
        ret += "\n  (no skills)";
    } else {
        for (const Skill* skill : this->force_remove_skills) {
            // TODO: Figure out a better solution to converting char* to std::string.
            ret += "\n  " + std::string(skill->name);
        }
    }

    ret += "\n\nForce-remove set bonuses:";
    if (set_bonus_cutoffs.size() == 0) {
        ret += "\n  (no set bonuses)";
    } else {
        for (const auto& e : this->set_bonus_cutoffs) {
            // TODO: Figure out a better solution to converting char* to std::string.
            ret += "\n  " + std::string(e.first->name) + " @ " + std::to_string(e.second) + " or more pieces";
        }
    }

    return ret;
}


bool SkillSpec::data_is_valid() const {
    // Check 1: All minimum levels fall within limits.
    for (const auto& e : this->min_levels) {
        const unsigned int limit = e.first->secret_limit;
        const unsigned int min_level = e.second;
        if (min_level > limit) return false;
    }
    // Check 2: All skills with states are in the min levels map, and are also within limits.
    for (const auto& e : this->states) {
        if (!Utils::map_has_key(this->min_levels, e.first)) return false;
        const unsigned int possible_states = e.first->states;
        const unsigned int state = e.second;
        if (state >= possible_states) return false;
    }
    if (this->min_levels.size() != this->states.size()) {
        return false;
    }
    // Check 3: No skills that are forced to be removed are present in the to-be-removed map.
    for (const auto& e : this->force_remove_skills) {
        if (Utils::map_has_key(this->min_levels, e)) return false;
    }
    // TODO: Can we check if any set bonus maximums prevent certain skills with minimum levels from ever being provided?

    // All checks passed. State is deemed valid.
    return true;
}


} // namespace

