/*
 * File: database.h
 * Author: <contact@simshadows.com>
 */

#ifndef MHWIBS_DATABASE_H
#define MHWIBS_DATABASE_H

#include <array>
#include <vector>


namespace Database {


/****************************************************************************************
 * Skills Database
 ***************************************************************************************/


constexpr unsigned int k_HANDICRAFT_MAX = 5;


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
    std::array<unsigned int, k_SHARPNESS_LEVELS> hits {};
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
    std::string    id; // The "UPPER_SNAKE_CASE" identifier of a weapon.

    WeaponClass    weapon_class;

    std::string    name; // Actual name, as it appears in-game.
    unsigned int   rarity;
    unsigned int   true_raw; // Must be converted from its bloated raw.
    int            affinity;

    bool           is_raw; // Temporary implementation while I sort out how to do elemental calculations.

    std::vector<unsigned int> deco_slots;

    std::string    skill_tmp;

    std::string    augmentation_scheme;
    std::string    upgrade_scheme;

    SharpnessGauge maximum_sharpness;
    bool           is_constant_sharpness;
};


struct WeaponsDatabase {
    // Field
    std::vector<Weapon> all_weapons {};

    // Constructor
    static const WeaponsDatabase read_weapon_db_file(const std::string& filename);

    // Access
    const Weapon* at(const std::string& weapon_id) const;

private:
    WeaponsDatabase() = default;
};


/****************************************************************************************
 * Database Manager
 ***************************************************************************************/

struct Database {
    // Field
    const WeaponsDatabase weapons {};

    // Constructor
    static const Database get_db();

private:
    // Constructor
    Database() noexcept;
};


} // namespace

#endif // MHWIBS_DATABASE_H

