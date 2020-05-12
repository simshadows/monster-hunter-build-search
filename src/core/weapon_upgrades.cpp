/*
 * File: weapon_upgrades.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include "core.h"
#include "../utils.h"

namespace MHWIBuildSearch
{


static const std::unordered_map<unsigned int, unsigned int> ib_custom_attack = {
    {0, 1},
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
    {5, 1},
    {6, 2},
};

static const std::unordered_map<unsigned int, int> ib_custom_affinity = {
    {0, 1},
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
    //{5, 0}, // Not Present
    {6, 3},
};


class NoWeaponUpgrades : public WeaponUpgradesInstance {
    const Weapon * const weapon;
public:
    NoWeaponUpgrades(const Weapon * const new_weapon) noexcept
        : weapon (new_weapon)
    {
    }

    WeaponUpgradesContribution calculate_contribution() const {
        return {0, 0, 0, weapon->maximum_sharpness, nullptr};
    }

    std::string get_humanreadable() const {
        return "Weapon upgrades:\n  (This weapon cannot be upgraded.)";
    }

    void add_upgrade(WeaponUpgrade) {
        throw std::logic_error("Attempted to upgrade a weapon that cannot be upgraded.");
    }
};


class IBCustomWeaponUpgrades : public WeaponUpgradesInstance {
    const Weapon * const weapon;
    std::vector<WeaponUpgrade> upgrades;
public:
    IBCustomWeaponUpgrades(const Weapon * const new_weapon) noexcept
        : weapon (new_weapon)
        , upgrades {}
    {
    }

    WeaponUpgradesContribution calculate_contribution() const {
        WeaponUpgradesContribution ret = {0, 0, 0, weapon->maximum_sharpness, nullptr};

        for (std::size_t i = 0; i < this->upgrades.size(); ++i) {
            switch (this->upgrades[i]) {
                case WeaponUpgrade::ib_cust_attack:
                    ret.added_raw += ib_custom_attack.at(i);
                    break;
                case WeaponUpgrade::ib_cust_affinity:
                    ret.added_aff += ib_custom_affinity.at(i);
                    break;
                default:
                    throw std::logic_error("Attempted to use an unsupported upgrade.");
            }
        }
        
        return ret;
    }

    std::string get_humanreadable() const {
        std::string ret = "Weapon upgrades:";
        if (this->upgrades.size() == 0) {
            ret += "\n  (no upgrades)";
        } else {
            for (std::size_t i = 0; i < this->upgrades.size(); ++i) {
                switch (this->upgrades[i]) {
                    case WeaponUpgrade::ib_cust_attack:
                        ret += "\n  Attack ";
                        break;
                    case WeaponUpgrade::ib_cust_affinity:
                        ret += "\n  Affinity ";
                        break;
                    default:
                        throw std::logic_error("Attempted to use an unsupported upgrade.");
                }
                ret += std::to_string(i + 1);
            }
        }
        return ret;
    }

    void add_upgrade(WeaponUpgrade upgrade) {
        // First, we test if it's valid.
        const std::size_t next_index = this->upgrades.size();
        switch (upgrade) {
            case WeaponUpgrade::ib_cust_attack:
                if (!Utils::map_has_key(ib_custom_attack, next_index)) {
                    throw InvalidChange("Attack upgrade cannot be applied at this time..");
                }
                break;
            case WeaponUpgrade::ib_cust_affinity:
                if (!Utils::map_has_key(ib_custom_affinity, next_index)) {
                    throw InvalidChange("Affinity upgrade cannot be applied at this time..");
                }
                break;
            default:
                throw InvalidChange("Attempted to apply an unsupported weapon upgrade.");
        }

        // Now, we add it!
        this->upgrades.emplace_back(upgrade);
    }
};


std::unique_ptr<WeaponUpgradesInstance> WeaponUpgradesInstance::get_instance(const Weapon * const weapon) {
    switch (weapon->upgrade_scheme) {
        case WeaponUpgradeScheme::none:            return std::make_unique<NoWeaponUpgrades>(weapon);
        case WeaponUpgradeScheme::iceborne_custom: return std::make_unique<IBCustomWeaponUpgrades>(weapon);
        case WeaponUpgradeScheme::iceborne_safi:   return std::make_unique<NoWeaponUpgrades>(weapon);
        default:
            throw std::runtime_error("This weapon's augmentation type is unsupported.");
    }
}


} // namespace

