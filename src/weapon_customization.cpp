/*
 * File: weapon_customization.cpp
 * Author: <contact@simshadows.com>
 */

#include "weapon_customization.h"

namespace MHWIBuildSearch
{


/****************************************************************************************
 * WeaponAugmentsInstance
 ***************************************************************************************/


class NoWeaponAugments : public WeaponAugmentsInstance {
public:
    NoWeaponAugments() noexcept = default;

    WeaponAugmentsContribution calculate_contribution() const {
        return {0, 0, 0, false};
    }
    
    std::string get_humanreadable() const {
        return "Weapon augments:\n  [no augments]";
    }
};


class IBWeaponAugments : public WeaponAugmentsInstance {
public:
    IBWeaponAugments(const Database::Weapon& weapon) noexcept {
        (void)weapon;
    }

    WeaponAugmentsContribution calculate_contribution() const {
        return {0, 10, 0, false};
    }
    
    std::string get_humanreadable() const {
        return "Weapon augments:\n  [flat 10 affinity]";
    }
};


std::unique_ptr<WeaponAugmentsInstance> WeaponAugmentsInstance::get_instance(const Database::Weapon& weapon) {
    switch (weapon.augmentation_scheme) {
        case Database::WeaponAugmentationScheme::none:     return std::make_unique<NoWeaponAugments>();
        case Database::WeaponAugmentationScheme::iceborne: return std::make_unique<IBWeaponAugments>(weapon);
        default:
            throw std::runtime_error("This weapon's augmentation type is unsupported.");
    }
}


} // namespace

