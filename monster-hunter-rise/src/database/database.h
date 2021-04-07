/*
 * File: database.h
 * Author: <contact@simshadows.com>
 */

#ifndef MHWIBS_DATABASE_H
#define MHWIBS_DATABASE_H

#include <map>
#include <unordered_set>
#include <unordered_map>

#include "../core/core.h"


namespace MHWIBuildSearch {


/****************************************************************************************
 * Decorations Database
 ***************************************************************************************/


class DecorationsDatabase {
    // TODO: Make this std::unique_ptr<...> somehow. I don't see why the compilers keep
    //       complaining about a call to an implicitly deleted copy constructor because
    //       I don't see why a copy constructor is even needed.
    std::unordered_map<std::string, std::shared_ptr<Decoration>> decorations_store;
public:
    // Constructor
    static const DecorationsDatabase read_db_file(const std::string& filename);

    // Access
    const Decoration* at(const std::string& deco_id) const;
    std::vector<const Decoration*> get_all() const;
    std::unordered_set<const Skill*> all_possible_skills_from_decos() const;

private:
    DecorationsDatabase() noexcept;
};


/****************************************************************************************
 * Weapons Database
 ***************************************************************************************/


class WeaponsDatabase {
    std::vector<Weapon> all_weapons;
public:
    // Constructor
    static const WeaponsDatabase read_db_file(const std::string& filename);

    // Access
    const Weapon* at(const std::string& weapon_id) const;
    std::vector<const Weapon*> get_all() const;
    std::vector<const Weapon*> get_all_of_weaponclass(WeaponClass) const;

private:
    WeaponsDatabase() noexcept;
};


/****************************************************************************************
 * Armour Database
 ***************************************************************************************/


// Reads string representations in UPPER_SNAKE_CASE (e.g. "MASTER_RANK") into tiers.
// Throws an exception if the string does not convert into a tier.

ArmourSlot upper_case_to_armour_slot(const std::string&);

ArmourVariant upper_snake_case_to_armour_variant(const std::string&);

Tier upper_snake_case_to_tier(const std::string&);


class ArmourDatabase {
    // Using a map to avoid having to implement a specialized hash function for unordered_map.
    // (Unlikely to need performance here anyway.)
    std::map<std::pair<std::string, Tier>, std::shared_ptr<ArmourSet>> armour_sets;
public:
    // Constructor
    static const ArmourDatabase read_db_file(const std::string& filename);

    // Access
    const ArmourPiece* at(const std::string& set_name,
                          Tier               tier,
                          ArmourVariant      variant,
                          ArmourSlot         slot) const;
    std::vector<const ArmourPiece*> get_all_pieces() const;
    std::map<ArmourSlot, std::vector<const ArmourPiece*>> get_all_pieces_by_slot() const;
    std::unordered_set<const Skill*> all_possible_skills_from_armour_without_set_bonuses() const;

private:
    ArmourDatabase() noexcept;
};


/****************************************************************************************
 * Charms Database
 ***************************************************************************************/


class CharmsDatabase {
    std::unordered_map<std::string, std::shared_ptr<Charm>> charms_map;
public:
    // Constructor
    static const CharmsDatabase read_db_file(const std::string& filename);

    // Access
    const Charm* at(const std::string& charm_id) const;
    std::vector<const Charm*> get_all() const;
    std::unordered_set<const Skill*> all_possible_skills_from_charms() const;

private:
    CharmsDatabase() noexcept;
};


/****************************************************************************************
 * Database Manager
 ***************************************************************************************/


struct Database {
    const DecorationsDatabase decos;
    const WeaponsDatabase     weapons;
    const ArmourDatabase      armour;
    const CharmsDatabase      charms;

    // Constructor
    static const Database get_db();

private:
    // Constructor
    Database();
};


} // namespace

#endif // MHWIBS_DATABASE_H

