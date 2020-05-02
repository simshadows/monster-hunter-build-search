/*
 * File: database_weapons.cpp
 * Author: <contact@simshadows.com>
 */

#include <array>

#include "database_skills.h"


namespace Weapons {


constexpr double k_GREATSWORD_BLOAT       = 4.8;
constexpr double k_LONGSWORD_BLOAT        = 3.3;
constexpr double k_SWORD_AND_SHIELD_BLOAT = 1.4;
constexpr double k_DUAL_BLADES_BLOAT      = 1.4;
constexpr double k_HAMMER_BLOAT           = 5.2;
constexpr double k_HUNTING_HORN_BLOAT     = 4.2;
constexpr double k_LANCE_BLOAT            = 2.3;
constexpr double k_GUNLANCE_BLOAT         = 2.3;
constexpr double k_SWITCHAXE_BLOAT        = 3.5;
constexpr double k_CHARGE_BLADE_BLOAT     = 3.6;
constexpr double k_INSECT_GLAIVE_BLOAT    = 4.1;
constexpr double k_BOW_BLOAT              = 1.2;
constexpr double k_HEAVY_BOWGUN_BLOAT     = 1.5;
constexpr double k_LIGHT_BOWGUN_BLOAT     = 1.3;


enum class WeaponType {
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


double weapontype_to_bloat_value(WeaponType);


constexpr std::size_t k_SHARPNESS_LEVELS = 7;

constexpr double k_RAW_SHARPNESS_MODIFIER_RED    = 0.50;
constexpr double k_RAW_SHARPNESS_MODIFIER_ORANGE = 0.75;
constexpr double k_RAW_SHARPNESS_MODIFIER_YELLOW = 1.00;
constexpr double k_RAW_SHARPNESS_MODIFIER_GREEN  = 1.05;
constexpr double k_RAW_SHARPNESS_MODIFIER_BLUE   = 1.20;
constexpr double k_RAW_SHARPNESS_MODIFIER_WHITE  = 1.32;
constexpr double k_RAW_SHARPNESS_MODIFIER_PURPLE = 1.39;


class SharpnessGauge {
    std::array<unsigned int, k_SHARPNESS_LEVELS> hits {};
public:

    // Constructor
    SharpnessGauge(unsigned int r,
                   unsigned int o,
                   unsigned int y,
                   unsigned int g,
                   unsigned int b,
                   unsigned int w,
                   unsigned int p) noexcept;

    // Constructs a new SharpnessGauge that is the result of applying handicraft to
    // another SharpnessGauge.
    SharpnessGauge apply_handicraft(unsigned int handicraft_lvl) noexcept;

    double get_raw_sharpness_modifier() const;

    std::string get_humanreadable() const;

protected:

    // Minimal constructor.
    // Doesn't do more work than is necessary.
    //
    // TODO: Make sure that it really doesn't do too much work, like initializing all
    //       array elements to zeroes.
    // TODO: Will `SharpnessGauge() = default` implement a minimal constructor?
    //       That would be preferable over a more explicit contructor definition.
    SharpnessGauge() noexcept;
    
};


} // namespace

