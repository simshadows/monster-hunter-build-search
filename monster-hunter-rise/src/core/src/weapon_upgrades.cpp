/*
 * File: weapon_upgrades.cpp
 * Author: <contact@simshadows.com>
 */

#include <stdexcept>
#include <assert.h>

#include "../core.h"
#include "../../utils/utils.h"

namespace MHRBuildSearch
{


/****************************************************************************************
 * NoWeaponUpgrades
 ***************************************************************************************/


class NoWeaponUpgrades : public WeaponUpgradesInstance {
    const Weapon * const weapon;
public:
    NoWeaponUpgrades(const Weapon * const new_weapon) noexcept
        : weapon (new_weapon)
    {
    }

    static std::vector<std::shared_ptr<WeaponUpgradesInstance>> generate_maximized_instances(const Weapon * const new_weapon) {
        return {std::make_shared<NoWeaponUpgrades>(new_weapon)};
    }

    WeaponUpgradesContribution calculate_contribution() const {
        return {0, 0, 0, 0, weapon->maximum_sharpness, nullptr};
    }

    std::string get_humanreadable() const {
        return "Weapon upgrades:\n    (This weapon cannot be upgraded.)";
    }

    void add_upgrade(WeaponUpgrade) {
        throw std::logic_error("Attempted to upgrade a weapon that cannot be upgraded.");
    }
};


/****************************************************************************************
 ***************************************************************************************/


std::shared_ptr<WeaponUpgradesInstance> WeaponUpgradesInstance::get_instance(const Weapon * const weapon) {
    switch (weapon->upgrade_scheme) {
        case WeaponUpgradeScheme::none:            return std::make_shared<NoWeaponUpgrades>(weapon);
        default:
            throw std::runtime_error("This weapon's upgrade type is unsupported.");
    }
}


std::vector<std::shared_ptr<WeaponUpgradesInstance>> WeaponUpgradesInstance::generate_maximized_instances(const Weapon * const weapon) {
    switch (weapon->upgrade_scheme) {
        case WeaponUpgradeScheme::none:            return NoWeaponUpgrades::generate_maximized_instances(weapon);
        default:
            throw std::runtime_error("This weapon's upgrade type is unsupported.");
    }
}


} // namespace

