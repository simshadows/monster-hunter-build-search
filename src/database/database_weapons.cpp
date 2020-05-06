/*
 * File: database_weapons.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <fstream>

#include "../../dependencies/json-3-7-3/json.hpp"

#include "database.h"


namespace Database {


/*
 * Static Stuff
 */


static const std::unordered_map<std::string, WeaponClass> str_to_weaponclass = {
    {"GREATSWORD"      , WeaponClass::greatsword      },
    {"LONGSWORD"       , WeaponClass::longsword       },
    {"SWORD_AND_SHIELD", WeaponClass::sword_and_shield},
    {"DUAL_BLADES"     , WeaponClass::dual_blades     },
    {"HAMMER"          , WeaponClass::hammer          },
    {"HUNTING_HORN"    , WeaponClass::hunting_horn    },
    {"LANCE"           , WeaponClass::lance           },
    {"GUNLANCE"        , WeaponClass::gunlance        },
    {"SWITCHAXE"       , WeaponClass::switchaxe       },
    {"CHARGE_BLADE"    , WeaponClass::charge_blade    },
    {"INSECT_GLAIVE"   , WeaponClass::insect_glaive   },
    {"BOW"             , WeaponClass::bow             },
    {"HEAVY_BOWGUN"    , WeaponClass::heavy_bowgun    },
    {"LIGHT_BOWGUN"    , WeaponClass::light_bowgun    },
};

static constexpr double k_GREATSWORD_BLOAT       = 4.8;
static constexpr double k_LONGSWORD_BLOAT        = 3.3;
static constexpr double k_SWORD_AND_SHIELD_BLOAT = 1.4;
static constexpr double k_DUAL_BLADES_BLOAT      = 1.4;
static constexpr double k_HAMMER_BLOAT           = 5.2;
static constexpr double k_HUNTING_HORN_BLOAT     = 4.2;
static constexpr double k_LANCE_BLOAT            = 2.3;
static constexpr double k_GUNLANCE_BLOAT         = 2.3;
static constexpr double k_SWITCHAXE_BLOAT        = 3.5;
static constexpr double k_CHARGE_BLADE_BLOAT     = 3.6;
static constexpr double k_INSECT_GLAIVE_BLOAT    = 4.1;
static constexpr double k_BOW_BLOAT              = 1.2;
static constexpr double k_HEAVY_BOWGUN_BLOAT     = 1.5;
static constexpr double k_LIGHT_BOWGUN_BLOAT     = 1.3;

static double weaponclass_to_bloat_value(const WeaponClass wt) {
    switch (wt) {
        case WeaponClass::greatsword:       return k_GREATSWORD_BLOAT;
        case WeaponClass::longsword:        return k_LONGSWORD_BLOAT;
        case WeaponClass::sword_and_shield: return k_SWORD_AND_SHIELD_BLOAT;
        case WeaponClass::dual_blades:      return k_DUAL_BLADES_BLOAT;
        case WeaponClass::hammer:           return k_HAMMER_BLOAT;
        case WeaponClass::hunting_horn:     return k_HUNTING_HORN_BLOAT;
        case WeaponClass::lance:            return k_LANCE_BLOAT;
        case WeaponClass::gunlance:         return k_GUNLANCE_BLOAT;
        case WeaponClass::switchaxe:        return k_SWITCHAXE_BLOAT;
        case WeaponClass::charge_blade:     return k_CHARGE_BLADE_BLOAT;
        case WeaponClass::insect_glaive:    return k_INSECT_GLAIVE_BLOAT;
        case WeaponClass::bow:              return k_BOW_BLOAT;
        case WeaponClass::heavy_bowgun:     return k_HEAVY_BOWGUN_BLOAT;
        case WeaponClass::light_bowgun:     return k_LIGHT_BOWGUN_BLOAT;
        default:
            throw "invalid weapon class";
    }
}

static constexpr double k_RAW_SHARPNESS_MODIFIER_RED    = 0.50;
static constexpr double k_RAW_SHARPNESS_MODIFIER_ORANGE = 0.75;
static constexpr double k_RAW_SHARPNESS_MODIFIER_YELLOW = 1.00;
static constexpr double k_RAW_SHARPNESS_MODIFIER_GREEN  = 1.05;
static constexpr double k_RAW_SHARPNESS_MODIFIER_BLUE   = 1.20;
static constexpr double k_RAW_SHARPNESS_MODIFIER_WHITE  = 1.32;
static constexpr double k_RAW_SHARPNESS_MODIFIER_PURPLE = 1.39;


/*
 * Header Implementations
 */


SharpnessGauge::SharpnessGauge(unsigned int r,
                               unsigned int o,
                               unsigned int y,
                               unsigned int g,
                               unsigned int b,
                               unsigned int w,
                               unsigned int p) noexcept
    : hits (std::array<unsigned int, k_SHARPNESS_LEVELS> {r, o, y, g, b, w, p})
{
    // Nothing left to do.
}


SharpnessGauge SharpnessGauge::from_vector(const std::vector<unsigned int>& vec) {
    SharpnessGauge ret;
    if (vec.size() != ret.hits.size()) {
        throw std::runtime_error("maximum_sharpness key from input json is an incorrect size.");
    }
    auto hits_end = std::copy(vec.begin(), vec.end(), ret.hits.begin());
    assert(hits_end == ret.hits.end()); // Sanity check
    return ret;
}


SharpnessGauge SharpnessGauge::apply_handicraft(unsigned int handicraft_lvl) const noexcept {
    assert(handicraft_lvl <= k_HANDICRAFT_MAX);

    SharpnessGauge ret;

    unsigned int hits_to_subtract = (k_HANDICRAFT_MAX - handicraft_lvl) * 10;

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
double SharpnessGauge::get_raw_sharpness_modifier() const {
    // TODO: Rewrite this unsafe reverse loop.
    for (int i = this->hits.size() - 1; i >= 0; --i) {
        if (this->hits[i] > 0) {
            switch (i) {
                case 0: return k_RAW_SHARPNESS_MODIFIER_RED;
                case 1: return k_RAW_SHARPNESS_MODIFIER_ORANGE;
                case 2: return k_RAW_SHARPNESS_MODIFIER_YELLOW;
                case 3: return k_RAW_SHARPNESS_MODIFIER_GREEN;
                case 4: return k_RAW_SHARPNESS_MODIFIER_BLUE;
                case 5: return k_RAW_SHARPNESS_MODIFIER_WHITE;
                case 6: return k_RAW_SHARPNESS_MODIFIER_PURPLE;
                default:
                    throw std::logic_error("Invalid index.");
            }
        }
    }
    // If the sharpness gauge is ever empty, we will still be in red gauge.
    return k_RAW_SHARPNESS_MODIFIER_RED;
}


// TODO: A simple, temporary implementation for now. Reimplement later if performance is required.
double SharpnessGauge::get_raw_sharpness_modifier(unsigned int handicraft_lvl) const {
    const SharpnessGauge modified_gauge = this->apply_handicraft(handicraft_lvl);
    return modified_gauge.get_raw_sharpness_modifier();
}


std::string SharpnessGauge::get_humanreadable() const {
    std::string buf = "";
    bool first = true;
    for (unsigned int v : this->hits) {
        if (!first) buf += " ";
        buf += std::to_string(v);
        first = false;
    }
    return buf;
}


SharpnessGauge::SharpnessGauge() noexcept
    : hits (std::array<unsigned int, k_SHARPNESS_LEVELS>())
{
}


// static
const WeaponsDatabase WeaponsDatabase::read_db_file(const std::string& filename, const SkillsDatabase& skills_db) {
    WeaponsDatabase new_db;
    new_db.all_weapons = std::vector<Weapon>(); // Do I need to do this?

    nlohmann::json j;

    {
        std::ifstream f(filename); // open file
        f >> j;
    } // close file

    assert(j.is_object());

    // TODO: I don't like how I don't know what the type of e is.
    for (auto& e : j.items()) {
        nlohmann::json jj(e.value());

        std::string weapon_id = e.key();
        if (!Utils::is_upper_snake_case(weapon_id)) {
            throw std::runtime_error("Weapon IDs in the weapon database must be in UPPER_SNAKE_CASE.");
        }

        WeaponClass weapon_class = str_to_weaponclass.at(jj["class"]);

        std::string name = jj["name"];
        unsigned int rarity = jj["rarity"];
        unsigned int bloated_raw = jj["attack"];
        unsigned int true_raw = bloated_raw / weaponclass_to_bloat_value(weapon_class);
        assert((true_raw * weaponclass_to_bloat_value(weapon_class)) == bloated_raw);
        int affinity = jj["affinity"];

        bool is_raw = jj["is_raw"];

        std::vector<unsigned int> deco_slots = jj["slots"];

        const Skill* skill;
        if (jj["skill"].is_null()) {
            skill = nullptr;
        } else {
            const std::string& skill_id = jj["skill"];
            skill = skills_db.skill_at(skill_id);
        }

        std::string augmentation_scheme = jj["augmentation_scheme"];
        std::string upgrade_scheme = jj["upgrade_scheme"];

        SharpnessGauge maximum_sharpness = SharpnessGauge::from_vector(jj["maximum_sharpness"]);
        bool is_constant_sharpness = jj["constant_sharpness"];

        // TODO: Ensure efficient construction of the Weapon object?
        //       I mean, inefficiency isn't so bad here, but let's try to do it right anyway lol
        new_db.all_weapons.push_back(Weapon {weapon_id,

                                             weapon_class,

                                             name,
                                             rarity,
                                             true_raw,
                                             affinity,

                                             is_raw,

                                             deco_slots,

                                             skill,

                                             augmentation_scheme,
                                             upgrade_scheme,

                                             maximum_sharpness,
                                             is_constant_sharpness});

    }

    return new_db;
}


const Weapon* WeaponsDatabase::at(const std::string& weapon_id) const {
    assert(Utils::is_upper_snake_case(weapon_id));
    for (const Weapon& e : this->all_weapons) {
        if (e.id == weapon_id) {
            return &e;
        }
    }
    throw std::runtime_error("No weapon exists with weapon ID '" + weapon_id + "'.");
}


WeaponsDatabase::WeaponsDatabase() noexcept = default;


} // namespace

