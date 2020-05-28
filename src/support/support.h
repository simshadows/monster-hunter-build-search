/*
 * File: support.h
 * Author: <contact@simshadows.com>
 */

#ifndef MHWIBS_SUPPORT_H
#define MHWIBS_SUPPORT_H

#include <unordered_map>

#include "../core/core.h"
#include "../database/database.h"
#include "../utils/counter.h"

namespace MHWIBuildSearch
{


// Declaring early since this will be needed.
class SkillMap;
class DecoEquips;


/****************************************************************************************
 * SkillSpec
 ***************************************************************************************/


class SkillSpec {
    using ContainerType = std::unordered_map<const Skill*, unsigned int>;

    ContainerType min_levels;
    ContainerType states;
public:
    using InputContainer    = ContainerType;
    using MinLevelsIterator = ContainerType::const_iterator;

    SkillSpec(InputContainer&& new_min_levels, InputContainer&& forced_states) noexcept;

    bool is_in_subset(const Skill*) const;
    unsigned int get_min_lvl(const Skill*) const;
    unsigned int get_state(const Skill*) const;
    bool get_state_for_binary_skill(const Skill*) const; // Adds an assertion for skills with only two states

    bool skills_meet_minimum_requirements(const SkillMap&) const;

    std::vector<const Skill*> get_skill_subset_as_vector() const;

    // Iterating is over the minimum levels. (We'll probably never need to iterate over the states anyway.)
    MinLevelsIterator begin() const;
    MinLevelsIterator end() const;

    void add_from_decos(const DecoEquips&);

    std::string get_humanreadable() const;

private:
    bool data_is_valid() const;
};


/****************************************************************************************
 * SetBonusMap
 ***************************************************************************************/


using SetBonusMap = Utils::CounterPKSV<const SetBonus*>;


/****************************************************************************************
 * SkillMap
 ***************************************************************************************/


struct HardClipSkillLevel {
    unsigned int operator()(const Skill * const skill, const unsigned int lvl) const noexcept {
        return (lvl > skill->secret_limit) ? skill->secret_limit : lvl;
    }
};


// Note that this container automatically clips levels to secret_limit.
class SkillMap : public Utils::CounterPKSV<const Skill*, HardClipSkillLevel> {
public:
    using Utils::CounterPKSV<const Skill*, HardClipSkillLevel>::CounterPKSV;

    SkillMap(const ArmourPiece&) noexcept;

    using Utils::CounterPKSV<const Skill*, HardClipSkillLevel>::merge_in;
    void merge_in(const std::vector<const Decoration*>&) noexcept; // Special case
    void merge_in(const DecoEquips&); // Special case

    void add_set_bonuses(const SetBonusMap&);

    // Only adds skills from the skill spec
    // Also adds an assertion to check that the skill map only contains skills from the skill spec
    void add_skills_filtered(const ArmourPiece&, const SkillSpec&);
    void add_skills_filtered(const Charm&, const unsigned int charm_lvl, const SkillSpec&);
    void add_skills_filtered(const Decoration&, const SkillSpec&);
    void add_skills_filtered(const std::vector<const Decoration*>&, const SkillSpec&);

    // Gets a skill's level. Skills that aren't in the container return zero.
    unsigned int get_non_secret(const Skill* skill, const Skill* associated_secret) const;
    bool is_at_least_lvl1(const Skill* skill) const;
    bool binary_skill_is_lvl1(const Skill* skill) const; // Adds an assertion for skills with only two levels.

    std::string get_humanreadable() const;

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
    SkillMap get_skills_without_set_bonuses_filtered(const SkillSpec&) const;
    SetBonusMap get_set_bonuses() const;
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
    unsigned int              weapon_raw {0};
    int                       weapon_aff {0};

    bool                      is_raw {false};

    std::vector<unsigned int> deco_slots {};
    const Skill*              skill      {nullptr};
    const SetBonus*           set_bonus  {nullptr};

    SharpnessGauge            maximum_sharpness     {0, 0, 0, 0, 0, 0, 0};
    bool                      is_constant_sharpness {false};

    bool                      health_regen_active {false};
};


struct WeaponInstance {
    const Weapon* weapon;

    // It is assumed that this field is constructed 
    std::shared_ptr<WeaponAugmentsInstance> augments;
    std::shared_ptr<WeaponUpgradesInstance> upgrades;

    WeaponInstance(const Weapon*) noexcept; // Generates a fresh weapon instance with no augments/upgrades.
    WeaponInstance(const Weapon*,
                   const std::shared_ptr<WeaponAugmentsInstance>&,
                   const std::shared_ptr<WeaponUpgradesInstance>&) noexcept; // Copies in augments and upgrades.

    WeaponContribution calculate_contribution() const;
    std::string get_humanreadable() const;
};


/****************************************************************************************
 * DecoEquips
 ***************************************************************************************/


class DecoEquips {
    using ContainerType = std::vector<const Decoration*>;
    using IteratorType  = ContainerType::const_iterator;

    ContainerType data;
public:
    DecoEquips() noexcept;
    DecoEquips(ContainerType&&) noexcept;

    bool fits_in(const ArmourEquips&, const WeaponContribution&) const;

    void add(const Decoration*);
    void merge_in(const DecoEquips&);

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

    SkillContribution(const SkillMap&,
                      const SkillSpec&,
                      const WeaponContribution&) noexcept;
};


/****************************************************************************************
 * Build Calculations
 ***************************************************************************************/


double calculate_efr_from_skills_lookup(const WeaponContribution&,
                                        const SkillMap&,
                                        const SkillSpec&);

double calculate_efr_from_gear_lookup(const WeaponInstance&,
                                      const ArmourEquips&,
                                      const DecoEquips&,
                                      const SkillSpec&);


} // namespace

#endif // MHWIBS_SUPPORT_H

