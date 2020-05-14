/*
 * File: database.h
 * Author: <contact@simshadows.com>
 */

#ifndef MHWIBS_DATABASE_H
#define MHWIBS_DATABASE_H

#include <map>
#include <unordered_map>

#include "../core/core.h"
#include "../utils/utils.h"


namespace MHWIBuildSearch {


/****************************************************************************************
 * Skills Database
 ***************************************************************************************/


class SkillsDatabase {
    // TODO: Make these std::unique_ptr<...> somehow. I don't see why the compilers keep
    //       complaining about a call to an implicitly deleted copy constructor because
    //       I don't see why a copy constructor is even needed.
    std::unordered_map<std::string, std::shared_ptr<Skill   >> skills_map;
    std::unordered_map<std::string, std::shared_ptr<SetBonus>> set_bonuses_map;
public:
    // Constructor
    static const SkillsDatabase read_db_file(const std::string& filename);

    // Access
    const Skill* skill_at(const std::string& skill_id) const;
    const SetBonus* set_bonus_at(const std::string& set_bonus_id) const;

    std::vector<const SetBonus*> get_all_set_bonuses() const;

private:
    SkillsDatabase() noexcept;
};


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
    static const DecorationsDatabase read_db_file(const std::string& filename, const SkillsDatabase& skills_db);

    // Access
    const Decoration* at(const std::string& deco_id) const;
    std::vector<const Decoration*> get_all() const;

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
    static const WeaponsDatabase read_db_file(const std::string& filename, const SkillsDatabase& skills_db);

    // Access
    const Weapon* at(const std::string& weapon_id) const;
    std::vector<const Weapon*> get_all() const;

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
    static const ArmourDatabase read_db_file(const std::string& filename, const SkillsDatabase& skills_db);

    // Access
    const ArmourPiece* at(const std::string& set_name,
                          Tier               tier,
                          ArmourVariant      variant,
                          ArmourSlot         slot) const;
    std::vector<const ArmourPiece*> get_all_pieces() const;

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
    static const CharmsDatabase read_db_file(const std::string& filename, const SkillsDatabase& skills_db);

    // Access
    const Charm* at(const std::string& charm_id) const;
    std::vector<const Charm*> get_all() const;

private:
    CharmsDatabase() noexcept;
};


/****************************************************************************************
 * Database Manager
 ***************************************************************************************/


struct Database {
    const SkillsDatabase      skills;
    const DecorationsDatabase decos;
    const WeaponsDatabase     weapons;
    const ArmourDatabase      armour;
    const CharmsDatabase      charms;

    // Pointers to skills with implemented features.
    // Used for high-performance comparisons without having to resort to reading hash tables.
    // (As a bonus, this will call out any important skills not present in the database.)
    const Skill* const affinity_sliding_ptr;
    const Skill* const agitator_ptr;
    const Skill* const agitator_secret_ptr;
    //const Skill* const airborne_ptr; // TODO: Figure out the math behind this, then implement it!
    const Skill* const attack_boost_ptr;
    //const Skill* const bludgeoner_ptr; // TODO: Figure out the math behind this, then implement it!
    const Skill* const coalescence_ptr;
    const Skill* const critical_boost_ptr;
    const Skill* const critical_draw_ptr;
    const Skill* const critical_eye_ptr;
    const Skill* const dragonvein_awakening_ptr;
    const Skill* const element_acceleration_ptr;
    //const Skill* const fortify_ptr; // TODO: Figure out the math behind this, then implement it!
    const Skill* const free_elem_ammo_up_ptr;
    //const Skill* const frostcraft_ptr; // TODO: Figure out the math behind this, then implement it!
    const Skill* const handicraft_ptr;
    //const Skill* const heroics_ptr; // TODO: Figure out the math behind this, then implement it!
    //const Skill* const heroics_secret_ptr;
    const Skill* const latent_power_ptr;
    const Skill* const latent_power_secret_ptr;
    const Skill* const maximum_might_ptr;
    const Skill* const maximum_might_secret_ptr;
    const Skill* const non_elemental_boost_ptr;
    //const Skill* const offensive_guard_ptr; // TODO: Figure out the math behind this, then implement it!
    const Skill* const peak_performance_ptr;
    //const Skill* const punishing_draw_ptr; // TODO: Figure out the math behind this, then implement it!
    const Skill* const resentment_ptr;
    const Skill* const true_dragonvein_awakening_ptr;
    const Skill* const true_element_acceleration_ptr;
    const Skill* const weakness_exploit_ptr;

    // Constructor
    static const Database get_db();

private:
    // Constructor
    Database();
};


} // namespace

#endif // MHWIBS_DATABASE_H

