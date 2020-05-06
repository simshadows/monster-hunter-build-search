/*
 * File: database.h
 * Author: <contact@simshadows.com>
 */

#ifndef MHWIBS_DATABASE_H
#define MHWIBS_DATABASE_H

#include <memory>
#include <array>
#include <vector>
#include <tuple>
#include <unordered_map>

#include "../utils.h"


namespace Database {


/****************************************************************************************
 * Skills Database
 ***************************************************************************************/


constexpr unsigned int k_HANDICRAFT_MAX = 5;


struct Skill {
    const std::string  id;            // The "UPPER_SNAKE_CASE" identifier of the skill.

    const std::string  name;          // Actual skill name, as it appears in-game.
    const unsigned int normal_limit;  // Maximum level without "secret" skills.
    const unsigned int secret_limit;  // Maximum level possible, usually only attainable with "secret" skills.
                                      // For programming convenience, normal_limit and secret_limit are equal
                                      // if no corresponding "secret" skill is available.

    const unsigned int states;        // Number of meaningful "states" that a skill has.
                                      // All skills have at least two states.
                                      // State 0 is always "off".
                                      // State 1 is always "on".
                                      // States beyond 1 are additional states that depend on the particular skill.
                                      // E.g. Weakness Exploit has three states depending on if you're hitting
                                      // a non-weakspot, a weakspot, or a tenderized weakspot.

    // The fields below are currently unused.

    //const std::string  tooltip;       // Tooltip for the skill, as it appears in-game.
    //const std::string  info;          // Additional information about a skill.

    //const std::string  previous_name; // Additional information about a skill.

    Skill(std::string  new_id,
          std::string  new_name,
          unsigned int new_normal_limit,
          unsigned int new_secret_limit,
          unsigned int new_states) noexcept;
};


struct SetBonus {
    const std::string id;
    const std::string name;
    const std::vector<std::tuple<unsigned int, const Skill*>> stages; // tuple format: (number of stages, skill)

    SetBonus(const std::string&& new_id,
             const std::string&& new_name,
             const std::vector<std::tuple<unsigned int, const Skill*>>&& new_stages) noexcept
        : id     (std::move(new_id    ))
        , name   (std::move(new_name  ))
        , stages (std::move(new_stages))
    {
    }
};


class SkillsDatabase {
    // TODO: Make these std::unique_ptr<...> somehow. I don't see why the compilers keep
    //       complaining about a call to an implicitly deleted copy constructor because
    //       I don't see why a copy constructor is even needed.
    std::unordered_map<std::string, std::shared_ptr<Skill   >> skills_map      {};
    std::unordered_map<std::string, std::shared_ptr<SetBonus>> set_bonuses_map {};
public:
    // Constructor
    static const SkillsDatabase read_db_file(const std::string& filename);

    // Access
    const Skill* skill_at(const std::string& skill_id) const;

private:
    SkillsDatabase() noexcept;
};


/****************************************************************************************
 * Weapons Database
 ***************************************************************************************/


enum class WeaponClass {
    greatsword,
    longsword,
    sword_and_shield,
    dual_blades,
    hammer,
    hunting_horn,
    lance,
    gunlance,
    switchaxe,
    charge_blade,
    insect_glaive,
    bow,
    heavy_bowgun,
    light_bowgun,
};


constexpr std::size_t k_SHARPNESS_LEVELS = 7;


class SharpnessGauge {
    std::array<unsigned int, k_SHARPNESS_LEVELS> hits;
public:

    SharpnessGauge(unsigned int r,
                   unsigned int o,
                   unsigned int y,
                   unsigned int g,
                   unsigned int b,
                   unsigned int w,
                   unsigned int p) noexcept;

    // Special constructor.
    // Throws an exception if the vector is of the wrong size.
    static SharpnessGauge from_vector(const std::vector<unsigned int>&);

    double get_raw_sharpness_modifier(unsigned int handicraft_lvl) const;

    std::string get_humanreadable() const;

protected:

    // Constructs a new SharpnessGauge that is the result of applying handicraft to
    // another SharpnessGauge.
    //
    // MOVE TO PUBLIC AS NEEDED.
    SharpnessGauge apply_handicraft(unsigned int handicraft_lvl) const noexcept;

    // MOVE TO PUBLIC AS NEEDED.
    double get_raw_sharpness_modifier() const;

    // Minimal constructor.
    // Doesn't do more work than is necessary.
    //
    // TODO: Make sure that it really doesn't do too much work, like initializing all
    //       array elements to zeroes.
    // TODO: Will `SharpnessGauge() = default` implement a minimal constructor?
    //       That would be preferable over a more explicit contructor definition.
    SharpnessGauge() noexcept;
    
};


struct Weapon {
    const std::string    id; // The "UPPER_SNAKE_CASE" identifier of the weapon.

    const WeaponClass    weapon_class;

    const std::string    name; // Actual name, as it appears in-game.
    const unsigned int   rarity;
    const unsigned int   true_raw; // Must be converted from its bloated raw.
    const int            affinity;

    const bool           is_raw; // Temporary implementation while I sort out how to do elemental calculations.

    const std::vector<unsigned int> deco_slots;

    const Skill*         skill; // nullptr if no skill.

    const std::string    augmentation_scheme;
    const std::string    upgrade_scheme;

    const SharpnessGauge maximum_sharpness;
    const bool           is_constant_sharpness;
};


class WeaponsDatabase {
    std::vector<Weapon> all_weapons;
public:
    // Constructor
    static const WeaponsDatabase read_db_file(const std::string& filename, const SkillsDatabase& skills_db);

    // Access
    const Weapon* at(const std::string& weapon_id) const;

private:
    WeaponsDatabase() noexcept;
};


/****************************************************************************************
 * Armour Database
 ***************************************************************************************/


class ArmourDatabase {
public:
    // Constructor
    static const ArmourDatabase read_db_file(const std::string& filename, const SkillsDatabase& skills_db);

private:
    ArmourDatabase() noexcept;
};


/****************************************************************************************
 * Database Manager
 ***************************************************************************************/


struct Database {
    const SkillsDatabase skills;
    const WeaponsDatabase weapons;
    const ArmourDatabase armour;

    // Pointers to skills with implemented features.
    // Used for high-performance comparisons without having to resort to reading hash tables.
    // (As a bonus, this will call out any important skills not present in the database.)
    const Skill* critical_boost_ptr;
    const Skill* handicraft_ptr;
    const Skill* non_elemental_boost_ptr;

    // Constructor
    static const Database get_db();

private:
    // Constructor
    Database();
};


} // namespace

#endif // MHWIBS_DATABASE_H

