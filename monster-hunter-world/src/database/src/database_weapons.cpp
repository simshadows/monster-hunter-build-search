/*
 * File: database_weapons.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <fstream>

#include "../../../dependencies/json-3-7-3/json.hpp"

#include "../database.h"
#include "../database_skills.h"
#include "../../utils/utils_strings.h"


namespace MHWIBuildSearch {


/*
 * Static Stuff
 */


static constexpr unsigned int k_ELESTAT_BLOAT_VALUE = 10;


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

static const std::unordered_map<std::string, WeaponUpgradeScheme> str_to_upgrade_scheme = {
    {"NONE"           , WeaponUpgradeScheme::none},
    {"ICEBORNE_CUSTOM", WeaponUpgradeScheme::iceborne_custom},
    {"ICEBORNE_SAFI"  , WeaponUpgradeScheme::iceborne_safi},
};

static const std::unordered_map<std::string, EleStatVisibility> str_to_elestat_visibility = {
    {"NONE",   EleStatVisibility::none},
    {"OPEN",   EleStatVisibility::open},
    {"HIDDEN", EleStatVisibility::hidden},
};

static const std::unordered_map<std::string, EleStatType> str_to_elestat_type = {
    // EleStatType::none is invalid here.

    {"FIRE",      EleStatType::fire     },
    {"WATER",     EleStatType::water    },
    {"THUNDER",   EleStatType::thunder  },
    {"ICE",       EleStatType::ice      },
    {"DRAGON",    EleStatType::dragon   },

    {"POISON",    EleStatType::poison   },
    {"SLEEP",     EleStatType::sleep    },
    {"PARALYSIS", EleStatType::paralysis},
    {"BLAST",     EleStatType::blast    },
};


// static
const WeaponsDatabase WeaponsDatabase::read_db_file(const std::string& filename) {
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

        WeaponClass weapon_class = upper_snake_case_to_weaponclass(jj["class"]);

        std::string name = jj["name"];
        unsigned int rarity = jj["rarity"];
        unsigned int bloated_raw = jj["attack"];
        unsigned int true_raw = bloated_raw / weaponclass_to_bloat_value(weapon_class);
        assert((true_raw * weaponclass_to_bloat_value(weapon_class)) == bloated_raw);
        int affinity = jj["affinity"];

        EleStatVisibility elestat_visibility = str_to_elestat_visibility.at(jj["elestat_visibility"]);
        EleStatType elestat_type;
        unsigned int elestat_value;
        if (elestat_visibility == EleStatVisibility::none) {
            elestat_type = EleStatType::none;
            elestat_value = 0;
        } else {
            elestat_type = str_to_elestat_type.at(jj["elestat_type"]);
            const unsigned int elestat_bloat_value = jj["elestat_value"];
            elestat_value = elestat_bloat_value / k_ELESTAT_BLOAT_VALUE;
            assert((elestat_value * k_ELESTAT_BLOAT_VALUE) == elestat_bloat_value);
            if (!elestat_value) {
                throw std::runtime_error("Element/status values must not be zero.");
            }
        }

        std::vector<unsigned int> deco_slots = jj["slots"];

        const Skill* skill;
        if (jj["skill"].is_null()) {
            skill = nullptr;
        } else {
            const std::string& skill_id = jj["skill"];
            skill = SkillsDatabase::get_skill(skill_id);
        }

        WeaponAugmentationScheme augmentation_scheme = str_to_augmentation_scheme.at(jj["augmentation_scheme"]);
        WeaponUpgradeScheme upgrade_scheme = str_to_upgrade_scheme.at(jj["upgrade_scheme"]);

        SharpnessGauge maximum_sharpness = SharpnessGauge::from_vector(jj["maximum_sharpness"]);
        bool is_constant_sharpness = jj["constant_sharpness"];

        // TODO: Ensure efficient construction of the Weapon object?
        //       I mean, inefficiency isn't so bad here, but let's try to do it right anyway lol
        new_db.all_weapons.push_back(Weapon {std::move(weapon_id),

                                             std::move(weapon_class),

                                             std::move(name),
                                             std::move(rarity),
                                             std::move(true_raw),
                                             std::move(affinity),

                                             std::move(elestat_visibility),
                                             std::move(elestat_type),
                                             std::move(elestat_value),

                                             std::move(deco_slots),

                                             std::move(skill),

                                             std::move(augmentation_scheme),
                                             std::move(upgrade_scheme),

                                             std::move(maximum_sharpness),
                                             std::move(is_constant_sharpness) });

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


std::vector<const Weapon*> WeaponsDatabase::get_all() const {
    std::vector<const Weapon*> ret;
    for (const Weapon& weapon : this->all_weapons) {
        ret.emplace_back(&weapon);
    }
    return ret;
}


std::vector<const Weapon*> WeaponsDatabase::get_all_of_weaponclass(WeaponClass weapon_class) const {
    std::vector<const Weapon*> ret;
    for (const Weapon& weapon : this->all_weapons) {
        if (weapon.weapon_class == weapon_class) ret.emplace_back(&weapon);
    }
    return ret;
}


WeaponsDatabase::WeaponsDatabase() noexcept = default;


} // namespace

