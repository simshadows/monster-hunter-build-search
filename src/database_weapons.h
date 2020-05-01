/*
 * File: database_weapons.h
 * Author: <contact@simshadows.com>
 */

//#include <cmath>
//#include <cstddef>
#include <array>
#include <assert.h>

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


double weapontype_to_bloat_value(WeaponType wt) {
    switch (wt) {
        case WeaponType::greatsword:
            return k_GREATSWORD_BLOAT;
        case WeaponType::longsword:
            return k_LONGSWORD_BLOAT;
        case WeaponType::sword_and_shield:
            return k_SWORD_AND_SHIELD_BLOAT;
        case WeaponType::dual_blades:
            return k_DUAL_BLADES_BLOAT;
        case WeaponType::hammer:
            return k_HAMMER_BLOAT;
        case WeaponType::hunting_horn:
            return k_HUNTING_HORN_BLOAT;
        case WeaponType::lance:
            return k_LANCE_BLOAT;
        case WeaponType::gunlance:
            return k_GUNLANCE_BLOAT;
        case WeaponType::switchaxe:
            return k_SWITCHAXE_BLOAT;
        case WeaponType::charge_blade:
            return k_CHARGE_BLADE_BLOAT;
        case WeaponType::insect_glaive:
            return k_INSECT_GLAIVE_BLOAT;
        case WeaponType::bow:
            return k_BOW_BLOAT;
        case WeaponType::heavy_bowgun:
            return k_HEAVY_BOWGUN_BLOAT;
        case WeaponType::light_bowgun:
            return k_LIGHT_BOWGUN_BLOAT;
        default:
            throw "invalid weapon type";
    }
}


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
                   unsigned int p) noexcept
        : hits (std::array<unsigned int, k_SHARPNESS_LEVELS> {r, o, y, g, b, w, p})
    {
    }

    // Constructs a new SharpnessGauge that is the result of applying handicraft to
    // another SharpnessGauge.
    SharpnessGauge apply_handicraft(unsigned int handicraft_lvl) noexcept {
        assert(handicraft_lvl <= Skills::k_HANDICRAFT_MAX);

        SharpnessGauge ret;

        unsigned int hits_to_subtract = (Skills::k_HANDICRAFT_MAX - handicraft_lvl) * 10;

        // TODO: Rewrite this unsafe reverse loop.
        assert(ret.hits.size() == this->hits.size());
        for (int i = ret.hits.size() - 1; i >= 0; --i) {
            if (this->hits[i] > hits_to_subtract) {
                ret.hits[i] = this->hits[i] - hits_to_subtract;
                hits_to_subtract = 0;
            } else {
                ret.hits[i] = 0;
                hits_to_subtract -= this->hits[i];
            }
        }

        return ret;
    }

    // TODO: Rewrite this function. It's so freaking ugly.
    double get_raw_sharpness_modifier() const {
        // TODO: Rewrite this unsafe reverse loop.
        for (int i = hits.size() - 1; i >= 0; --i) {
            if (hits[i] > 0) {
                switch (i) {
                    case 0:  return k_RAW_SHARPNESS_MODIFIER_RED;
                    case 1:  return k_RAW_SHARPNESS_MODIFIER_ORANGE;
                    case 2:  return k_RAW_SHARPNESS_MODIFIER_YELLOW;
                    case 3:  return k_RAW_SHARPNESS_MODIFIER_GREEN;
                    case 4:  return k_RAW_SHARPNESS_MODIFIER_BLUE;
                    case 5:  return k_RAW_SHARPNESS_MODIFIER_WHITE;
                    case 6:  return k_RAW_SHARPNESS_MODIFIER_PURPLE;
                    default: throw std::logic_error("Invalid index.");
                }
            }
        }
        // If the sharpness gauge is ever empty, we will still be in red gauge.
        return k_RAW_SHARPNESS_MODIFIER_RED;
    }

    std::string get_humanreadable() const {
        std::string buf = "";
        bool first = true;
        for (unsigned int v : hits) {
            if (!first) buf += " ";
            buf += std::to_string(v);
            first = false;
        }
        return buf;
    }

protected:

    // Minimal Constructor
    SharpnessGauge() noexcept
        : hits (std::array<unsigned int, k_SHARPNESS_LEVELS>())
    {
    }
    
};


} // namespace

