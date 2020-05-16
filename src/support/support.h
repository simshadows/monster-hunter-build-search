/*
 * File: support.h
 * Author: <contact@simshadows.com>
 */

#ifndef MHWIBS_SUPPORT_H
#define MHWIBS_SUPPORT_H

#include <unordered_map>

#include "../core/core.h"
#include "../database/database.h"

namespace MHWIBuildSearch
{


class DecoEquips; // Declaring early since this will be needed.


/****************************************************************************************
 * SkillSpec
 ***************************************************************************************/


class SkillSpec {
    typedef std::unordered_map<const Skill*, unsigned int> ContainerType;

    ContainerType min_levels;
    ContainerType states;
public:
    typedef ContainerType                 InputContainer;
    typedef ContainerType::const_iterator MinLevelsIterator;

    SkillSpec(InputContainer&& new_min_levels, InputContainer&& forced_states) noexcept;

    bool is_in_subset(const Skill*) const;
    unsigned int get_min_lvl(const Skill*) const;
    unsigned int get_state(const Skill*) const;
    bool get_state_for_binary_skill(const Skill*) const; // Adds an assertion for skills with only two states

    // Iterating is over the minimum levels. (We'll probably never need to iterate over the states anyway.)
    MinLevelsIterator begin() const;
    MinLevelsIterator end() const;

    void add_from_decos(const DecoEquips&);

    std::string get_humanreadable() const;

private:
    bool data_is_valid() const;
};


/****************************************************************************************
 * SkillMap
 ***************************************************************************************/


// Note that this container automatically clips levels to secret_limit.
class SkillMap {
    typedef std::unordered_map<const Skill*, unsigned int> ContainerType;
    typedef ContainerType::const_iterator IteratorType;

    ContainerType data;
public:
    SkillMap() noexcept;
    SkillMap(const ArmourPiece&) noexcept;

    void set_lvl(const Skill* skill, unsigned int level);
    void increment_lvl(const Skill* skill, unsigned int level_to_add);
    void decrement_lvl(const Skill* skill, unsigned int level_to_remove);
    void add_skills(const ArmourPiece&);
    void add_skills(const DecoEquips&);
    void add_set_bonuses(const std::unordered_map<const SetBonus*, unsigned int>&);
    void remove_skill(const Skill* skill);

    // Only adds skills from the skill spec
    // Also adds an assertion to check that the skill map only contains skills from the skill spec
    void add_skills_filtered(const ArmourPiece&, const SkillSpec&);
    void add_skills_filtered(const std::vector<const Decoration*>&, const SkillSpec&);

    // Gets a skill's level. Skills that aren't in the container return zero.
    unsigned int get_lvl(const Skill* skill) const;
    //unsigned int get_lvl_no_secret(const Skill* skill) const;
    unsigned int get_lvl(const Skill* skill, const Skill* associated_secret) const;
    bool is_at_least_lvl1(const Skill* skill) const;
    bool binary_skill_is_lvl1(const Skill* skill) const; // Adds an assertion for skills with only two levels.

    std::size_t calculate_hash() const noexcept;

    IteratorType begin() const;
    IteratorType end() const;

    std::string get_humanreadable() const;

    bool operator==(const SkillMap& x) const noexcept {
        return this->data == x.data;
    }
private:
    bool only_contains_skills_in_spec(const SkillSpec&) const noexcept;
};


/****************************************************************************************
 * ArmourEquips
 ***************************************************************************************/


// Charms will be considered an armour piece here, for algorithmic convenience.


class ArmourEquips {
    static constexpr std::size_t k_NUM_ARMOUR_SLOTS = 5;

    std::array<const ArmourPiece*, k_NUM_ARMOUR_SLOTS> data;

    const Charm* charm;
    //unsigned int charm_lvl; // For now, we will only allow constructing max-level charms.
public:
    ArmourEquips() noexcept;
    ArmourEquips(const Charm*) noexcept;
    //ArmourEquips(const std::initializer_list<const ArmourPiece>&) noexcept;

    void add(const ArmourPiece*);
    void add(const Charm*);

    bool slot_is_filled(const ArmourSlot&) const;
    SkillMap get_skills_without_set_bonuses() const;
    std::unordered_map<const SetBonus*, unsigned int> get_set_bonuses() const;
    std::vector<unsigned int> get_deco_slots() const;

    std::string get_humanreadable() const;

private:
    // TODO: This helper function is probably unnecessary. Try to get rid of it.
    static std::size_t slot_to_index(const ArmourSlot&);

    std::string fetch_piece_name(std::size_t) const;
    std::string fetch_charm_name() const;
};


/****************************************************************************************
 * WeaponInstance
 ***************************************************************************************/


struct WeaponContribution {
    unsigned int              weapon_raw;
    int                       weapon_aff;

    bool                      is_raw;

    std::vector<unsigned int> deco_slots;
    const Skill*              skill;
    const SetBonus*           set_bonus;

    SharpnessGauge            maximum_sharpness;
    bool                      is_constant_sharpness;

    bool                      health_regen_active;
};


struct WeaponInstance {
    const Weapon* weapon;

    // It is assumed that this field is constructed 
    std::unique_ptr<WeaponAugmentsInstance> augments;
    std::unique_ptr<WeaponUpgradesInstance> upgrades;

    WeaponInstance(const Weapon*) noexcept; // Generates a fresh weapon instance with no augments/upgrades.

    WeaponContribution calculate_contribution(const Database&) const;
    std::string get_humanreadable() const;
};


/****************************************************************************************
 * DecoEquips
 ***************************************************************************************/


class DecoEquips {
    typedef std::vector<const Decoration*> ContainerType;
    typedef ContainerType::const_iterator  IteratorType;

    ContainerType data;
public:
    DecoEquips() noexcept;
    DecoEquips(ContainerType&&) noexcept;

    bool fits_in(const ArmourEquips&, const WeaponContribution&) const;

    void add(const Decoration*);

    IteratorType begin() const;
    IteratorType end() const;

    std::string get_humanreadable() const;
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

    SkillContribution(const Database&,
                      const SkillMap&,
                      const SkillSpec&,
                      const WeaponContribution&) noexcept;
};


/****************************************************************************************
 * Build Calculations
 ***************************************************************************************/


double calculate_efr_from_skills_lookup(const Database&,
                                        const WeaponContribution&,
                                        const SkillMap&,
                                        const SkillSpec&);

double calculate_efr_from_gear_lookup(const Database&,
                                      const WeaponInstance&,
                                      const ArmourEquips&,
                                      const DecoEquips&,
                                      const SkillSpec&);


} // namespace

#endif // MHWIBS_SUPPORT_H

