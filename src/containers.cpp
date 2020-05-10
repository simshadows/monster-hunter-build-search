/*
 * File: containers.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include "containers.h"
#include "utils.h"


namespace MHWIBuildSearch
{


/****************************************************************************************
 * SkillSpec
 ***************************************************************************************/


static constexpr unsigned int k_DEACTIVATED_STATE = 0;


SkillSpec::SkillSpec(InputContainer&& new_min_levels, InputContainer&& forced_states) noexcept
    : min_levels (std::move(new_min_levels   ))
    , states     (std::move(forced_states))
{
    for (const auto& e : this->min_levels) {
        const Database::Skill * const skill = e.first;
        if (!Utils::map_has_key(this->states, skill)) {
            const unsigned int max_state = skill->states - 1;
            this->states.emplace(skill, max_state);
        }
    }
    assert(this->data_is_valid());
}


bool SkillSpec::is_in_subset(const Database::Skill* skill) const {
    return Utils::map_has_key(this->min_levels, skill);
}


unsigned int SkillSpec::get_min_lvl(const Database::Skill* skill) const {
    const auto& result = this->min_levels.find(skill);
    return (result == this->min_levels.end()) ? 0 : result->second;
}


unsigned int SkillSpec::get_state(const Database::Skill * const skill) const {
    const auto& result = this->states.find(skill);
    return (result == this->states.end()) ? k_DEACTIVATED_STATE : result->second;
}


bool SkillSpec::get_state_for_binary_skill(const Database::Skill * const skill) const {
    assert(skill->states == 2);
    const auto& result = this->states.find(skill);
    return result != this->states.end();
}


std::string SkillSpec::get_humanreadable() const {
    assert(this->data_is_valid());
    std::string ret = "Skill Subset:";

    if (min_levels.size() == 0) {
        ret += "\n  (no skills)";
    } else {
        for (const auto& e : this->min_levels) {
            const Database::Skill * const skill   = e.first;
            const unsigned int            min_lvl = e.second;
            const unsigned int            state   = this->states.at(skill);
            assert(state != 0);

            ret += "\n  " + skill->name;
            if (min_lvl > 0)       ret += " [Minimum level: " + std::to_string(min_lvl) + "]";
            if (skill->states > 2) ret += " [State: " + std::to_string(state) + "]";
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
    return this->min_levels.size() == this->states.size();
}


/****************************************************************************************
 * SkillMap
 ***************************************************************************************/


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


void SkillMap::add_skills(const Database::ArmourPiece& piece) {
    for (const std::pair<const Database::Skill*, unsigned int>& e : piece.skills) {
        this->increment_lvl(e.first, e.second);
    }
}


unsigned int SkillMap::get_lvl(const Database::Skill * const skill) const {
    assert(this->data.count(skill) <= 1); // count must return either 1 or 0.
    return this->data.count(skill) ? this->data.at(skill) : 0;
}


//unsigned int SkillMap::get_lvl_no_secret(const Database::Skill * const skill) const {
//    assert(this->data.count(skill) <= 1); // count must return either 1 or 0.
//    const unsigned int normal_limit = skill->normal_limit;
//    if (Utils::map_has_key(this->data, skill)) {
//        const unsigned int level = this->data.at(skill);
//        return (level > normal_limit) ? normal_limit : level;
//    } else {
//        return 0;
//    }
//}


unsigned int SkillMap::get_lvl(const Database::Skill * const skill,
                               const Database::Skill * const associated_secret) const {
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


bool SkillMap::binary_skill_is_lvl1(const Database::Skill* skill) const {
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


/****************************************************************************************
 * ArmourEquips
 ***************************************************************************************/


static constexpr std::size_t k_HEAD_INDEX  = 0;
static constexpr std::size_t k_CHEST_INDEX = 1;
static constexpr std::size_t k_ARMS_INDEX  = 2;
static constexpr std::size_t k_WAIST_INDEX = 3;
static constexpr std::size_t k_LEGS_INDEX  = 4;


ArmourEquips::ArmourEquips() noexcept
    : data  {}
    , charm {}
{
}


void ArmourEquips::add(const Database::ArmourPiece* piece) {
    this->data[slot_to_index(piece->slot)] = piece;
}


void ArmourEquips::add(const Database::Charm* new_charm) {
    this->charm = new_charm;
}


bool ArmourEquips::slot_is_filled(const Database::ArmourSlot& slot) const {
    return this->data[slot_to_index(slot)];
}


SkillMap ArmourEquips::get_skills_without_set_bonuses() const {
    SkillMap ret;
    for (const Database::ArmourPiece * const & armour_piece : this->data) {
        if (!armour_piece) continue;
        ret.add_skills(*armour_piece);
    }
    if (this->charm) {
        unsigned int charm_lvl = this->charm->max_charm_lvl;
        for (const Database::Skill* const& skill : this->charm->skills) {
            ret.increment_lvl(skill, charm_lvl);
        }
    }
    return ret;
}


std::string ArmourEquips::get_humanreadable() const {
    return "Head:  " + this->fetch_piece_name(k_HEAD_INDEX)
           + "\nChest: " + this->fetch_piece_name(k_CHEST_INDEX)
           + "\nArms:  " + this->fetch_piece_name(k_ARMS_INDEX)
           + "\nWaist: " + this->fetch_piece_name(k_WAIST_INDEX)
           + "\nLegs:  " + this->fetch_piece_name(k_LEGS_INDEX)
           + "\nCharm: " + this->fetch_charm_name();
}


std::size_t ArmourEquips::slot_to_index(const Database::ArmourSlot& slot) {
    switch (slot) {
        case Database::ArmourSlot::head:  return k_HEAD_INDEX;
        case Database::ArmourSlot::chest: return k_CHEST_INDEX;
        case Database::ArmourSlot::arms:  return k_ARMS_INDEX;
        case Database::ArmourSlot::waist: return k_WAIST_INDEX;
        case Database::ArmourSlot::legs:  return k_LEGS_INDEX;
        default:
            throw std::runtime_error("Invalid armour slot.");
    }
}


std::string ArmourEquips::fetch_piece_name(const std::size_t index) const {
    const Database::ArmourPiece * const & piece = this->data[index];
    if (piece) {
        return piece->get_full_name();
    } else {
        return "[empty]";
    }
}


std::string ArmourEquips::fetch_charm_name() const {
    if (this->charm) {
        return this->charm->name + " " + Utils::to_capital_roman_numerals(this->charm->max_charm_lvl);
    } else {
        return "[empty]";
    }
}


} // namespace

