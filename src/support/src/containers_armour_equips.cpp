/*
 * File: containers_armour_equips.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <algorithm>

#include "../support.h"
#include "../../utils/utils.h"


namespace MHWIBuildSearch
{


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


ArmourEquips::ArmourEquips(const Charm* new_charm) noexcept
    : data  {}
    , charm (new_charm)
{
}


void ArmourEquips::add(const ArmourPiece* piece) {
    this->data[slot_to_index(piece->slot)] = piece;
}


void ArmourEquips::add(const Charm* new_charm) {
    this->charm = new_charm;
}


bool ArmourEquips::slot_is_filled(const ArmourSlot& slot) const {
    return this->data[slot_to_index(slot)];
}


SkillMap ArmourEquips::get_skills_without_set_bonuses() const {
    SkillMap ret;
    for (const ArmourPiece * const & armour_piece : this->data) {
        if (armour_piece) ret.merge_in(armour_piece->skills);
    }
    if (this->charm) {
        unsigned int charm_lvl = this->charm->max_charm_lvl;
        for (const Skill* const& skill : this->charm->skills) {
            ret.increment(skill, charm_lvl);
        }
    }
    return ret;
}


SkillMap ArmourEquips::get_skills_without_set_bonuses_filtered(const SkillSpec& skill_spec) const {
    SkillMap ret;
    for (const ArmourPiece * const & armour_piece : this->data) {
        if (armour_piece) ret.add_skills_filtered(*armour_piece, skill_spec);
    }
    if (this->charm) {
        unsigned int charm_lvl = this->charm->max_charm_lvl;
        for (const Skill* const& skill : this->charm->skills) {
            if (skill_spec.is_in_subset(skill)) ret.increment(skill, charm_lvl);
        }
    }
    return ret;
}


std::unordered_map<const SetBonus*, unsigned int> ArmourEquips::get_set_bonuses() const {
    std::unordered_map<const SetBonus*, unsigned int> ret;
    for (const ArmourPiece * const e : this->data) {
        if (e && e->set_bonus) ret[e->set_bonus] += 1;
    }
    return ret;
}


std::vector<unsigned int> ArmourEquips::get_deco_slots() const {
    std::vector<unsigned int> ret;
    for (const ArmourPiece * const & armour_piece : this->data) {
        if (!armour_piece) continue;
        ret.insert(ret.end(), armour_piece->deco_slots.begin(), armour_piece->deco_slots.end());
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


std::size_t ArmourEquips::slot_to_index(const ArmourSlot& slot) {
    switch (slot) {
        case ArmourSlot::head:  return k_HEAD_INDEX;
        case ArmourSlot::chest: return k_CHEST_INDEX;
        case ArmourSlot::arms:  return k_ARMS_INDEX;
        case ArmourSlot::waist: return k_WAIST_INDEX;
        case ArmourSlot::legs:  return k_LEGS_INDEX;
        default:
            throw std::runtime_error("Invalid armour slot.");
    }
}


std::string ArmourEquips::fetch_piece_name(const std::size_t index) const {
    const ArmourPiece * const & piece = this->data[index];
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

