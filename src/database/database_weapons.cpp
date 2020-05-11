/*
 * File: database_weapons.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <fstream>

#include "../../dependencies/json-3-7-3/json.hpp"

#include "database.h"


namespace MHWIBuildSearch {


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

static double weaponclass_to_bloat_value(const WeaponClass wt) {
    switch (wt) {
        case WeaponClass::greatsword:       return 4.8;
        case WeaponClass::longsword:        return 3.3;
        case WeaponClass::sword_and_shield: return 1.4;
        case WeaponClass::dual_blades:      return 1.4;
        case WeaponClass::hammer:           return 5.2;
        case WeaponClass::hunting_horn:     return 4.2;
        case WeaponClass::lance:            return 2.3;
        case WeaponClass::gunlance:         return 2.3;
        case WeaponClass::switchaxe:        return 3.5;
        case WeaponClass::charge_blade:     return 3.6;
        case WeaponClass::insect_glaive:    return 4.1;
        case WeaponClass::bow:              return 1.2;
        case WeaponClass::heavy_bowgun:     return 1.5;
        case WeaponClass::light_bowgun:     return 1.3;
        default:
            throw "invalid weapon class";
    }
}

static const std::unordered_map<std::string, WeaponAugmentationScheme> str_to_augmentation_scheme = {
    {"NONE"     , WeaponAugmentationScheme::none},
    //{"BASE_GAME", WeaponAugmentationScheme::base_game}, // Currently unused.
    {"ICEBORNE" , WeaponAugmentationScheme::iceborne},
};


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

        WeaponAugmentationScheme augmentation_scheme = str_to_augmentation_scheme.at(jj["augmentation_scheme"]);
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

