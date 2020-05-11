/*
 * File: support.h
 * Author: <contact@simshadows.com>
 */

#ifndef MHWIBS_SUPPORT_H
#define MHWIBS_SUPPORT_H

#include <unordered_map>

#include "../database/database.h"

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
    bool get_state_for_binary_skill(const Database::Skill*) const; // Adds an assertion for skills with only two states

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
    bool binary_skill_is_lvl1(const Database::Skill* skill) const; // Adds an assertion for skills with only two levels.

    std::string get_humanreadable() const;
};


/****************************************************************************************
 * ArmourEquips
 ***************************************************************************************/


// Charms will be considered an armour piece here, for algorithmic convenience.


class ArmourEquips {
    static constexpr std::size_t k_NUM_ARMOUR_SLOTS = 5;

    std::array<const Database::ArmourPiece*, k_NUM_ARMOUR_SLOTS> data;

    const Database::Charm* charm;
    //unsigned int charm_lvl; // For now, we will only allow constructing max-level charms.
public:
    ArmourEquips() noexcept;
    //ArmourEquips(const std::initializer_list<const Database::ArmourPiece>&) noexcept;

    void add(const Database::ArmourPiece*);
    void add(const Database::Charm*);

    bool slot_is_filled(const Database::ArmourSlot&) const;
    SkillMap get_skills_without_set_bonuses() const;

    std::string get_humanreadable() const;

private:
    // TODO: This helper function is probably unnecessary. Try to get rid of it.
    static std::size_t slot_to_index(const Database::ArmourSlot&);

    std::string fetch_piece_name(std::size_t) const;
    std::string fetch_charm_name() const;
};


/****************************************************************************************
 * SkillContribution
 ***************************************************************************************/


struct SkillContribution {
    unsigned int added_raw;
    int          added_aff;
    double       neb_multiplier;
    double       raw_crit_dmg_multiplier;
    double       raw_sharpness_modifier;

    SkillContribution(const Database::Database&,
                      const SkillMap&,
                      const SkillSpec&,
                      const Database::Weapon&,
                      const Database::SharpnessGauge&) noexcept;
};


} // namespace

#endif // MHWIBS_SUPPORT_H

