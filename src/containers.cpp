/*
 * File: containers.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include "containers.h"


namespace MHWIBuildSearch
{


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


/****************************************************************************************
 * ArmourEquips
 ***************************************************************************************/


static constexpr std::size_t k_HEAD_INDEX  = 0;
static constexpr std::size_t k_CHEST_INDEX = 1;
static constexpr std::size_t k_ARMS_INDEX  = 2;
static constexpr std::size_t k_WAIST_INDEX = 3;
static constexpr std::size_t k_LEGS_INDEX  = 4;


ArmourEquips::ArmourEquips() noexcept {
    for (const Database::ArmourPiece*& e : this->data) {
        e = nullptr;
    }
}


void ArmourEquips::add(const Database::ArmourPiece * const & piece) {
    this->data[slot_to_index(piece->slot)] = piece;
}


bool ArmourEquips::slot_is_filled(const Database::ArmourSlot& slot) const {
    return this->data[slot_to_index(slot)];
}


std::string ArmourEquips::get_humanreadable() const {
    return "Head:  " + this->fetch_piece_name(k_HEAD_INDEX)
           + "\nChest: " + this->fetch_piece_name(k_CHEST_INDEX)
           + "\nArms:  " + this->fetch_piece_name(k_ARMS_INDEX)
           + "\nWaist: " + this->fetch_piece_name(k_WAIST_INDEX)
           + "\nLegs:  " + this->fetch_piece_name(k_LEGS_INDEX);
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


std::string ArmourEquips::fetch_piece_name(const std::size_t& index) const {
    const Database::ArmourPiece * const & piece = this->data[index];
    if (piece) {
        return piece->get_full_name();
    } else {
        return "[empty]";
    }
}


} // namespace

