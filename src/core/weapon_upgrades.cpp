/*
 * File: weapon_upgrades.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include "core.h"
#include "../utils.h"

namespace MHWIBuildSearch
{


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


std::unique_ptr<WeaponUpgradesInstance> WeaponUpgradesInstance::get_instance(const Weapon * const weapon) {
    switch (weapon->upgrade_scheme) {
        case WeaponUpgradeScheme::none:            return std::make_unique<NoWeaponUpgrades>(weapon);
        case WeaponUpgradeScheme::iceborne_custom: return std::make_unique<NoWeaponUpgrades>(weapon);
        case WeaponUpgradeScheme::iceborne_safi:   return std::make_unique<NoWeaponUpgrades>(weapon);
        default:
            throw std::runtime_error("This weapon's augmentation type is unsupported.");
    }
}


} // namespace

