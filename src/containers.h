/*
 * File: containers.h
 * Author: <contact@simshadows.com>
 */

#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <unordered_map>

#include "database/database.h"

namespace MHWIBuildSearch
{


/****************************************************************************************
 * SkillSpec
 ***************************************************************************************/


class SkillSpec {
    std::unordered_map<const Database::Skill*, unsigned int> min_levels;
    std::unordered_map<const Database::Skill*, unsigned int> states;
public:
    typedef std::unordered_map<const Database::Skill*, unsigned int> InputContainer;

    SkillSpec(InputContainer&& new_min_levels, InputContainer&& forced_states) noexcept;

    bool is_in_subset(const Database::Skill*) const;
    unsigned int get_min_lvl(const Database::Skill*) const;
    unsigned int get_state(const Database::Skill*) const;
    bool get_state_for_binary_skill(const Database::Skill*) const; // Adds an assertion

    std::string get_humanreadable() const;

private:
    bool data_is_valid() const;
};


/****************************************************************************************
 * SkillMap
 ***************************************************************************************/


// Note that this container automatically clips levels to secret_limit.
class SkillMap {
    std::unordered_map<const Database::Skill*, unsigned int> data;
public:
    SkillMap() noexcept;

    void set_lvl(const Database::Skill* skill, unsigned int level);
    void increment_lvl(const Database::Skill* skill, unsigned int level_to_add);
    void add_skills(const Database::ArmourPiece&);

    // Gets a skill's level. Skills that aren't in the container return zero.
    unsigned int get_lvl(const Database::Skill* skill) const;
    //unsigned int get_lvl_no_secret(const Database::Skill* skill) const;
    unsigned int get_lvl(const Database::Skill* skill, const Database::Skill* associated_secret) const;

    std::string get_humanreadable() const;
};


/****************************************************************************************
 * ArmourEquips
 ***************************************************************************************/


class ArmourEquips {
    static constexpr std::size_t k_NUM_ARMOUR_SLOTS = 5;

    std::array<const Database::ArmourPiece*, k_NUM_ARMOUR_SLOTS> data;
public:
    ArmourEquips() noexcept;
    ArmourEquips(const std::initializer_list<const Database::ArmourPiece>&) noexcept;

    void add(const Database::ArmourPiece*);

    bool slot_is_filled(const Database::ArmourSlot&) const;
    SkillMap get_skills_without_set_bonuses() const;

    std::string get_humanreadable() const;

private:
    // TODO: This helper function is probably unnecessary. Try to get rid of it.
    static std::size_t slot_to_index(const Database::ArmourSlot&);

    std::string fetch_piece_name(std::size_t) const;
};


} // namespace

#endif // CONTAINERS_H

