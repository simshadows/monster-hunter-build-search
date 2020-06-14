/*
 * File: containers_weapon_instance.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <algorithm>

#include "../../database/database_skills.h"
#include "../support.h"
#include "../../utils/utils.h"


namespace MHWIBuildSearch
{


void WeaponContribution::erase_elestat() noexcept {
    //this->elestat_visibility = EleStatVisibility::none;
    //this->elestat_type = EleStatType::none;
    this->elestat_value = 0;
}


WeaponInstance::WeaponInstance(const Weapon * const new_weapon) noexcept
    : weapon   (new_weapon)
    , augments (WeaponAugmentsInstance::get_instance(weapon))
    , upgrades (WeaponUpgradesInstance::get_instance(weapon))
{
}


WeaponInstance::WeaponInstance(const Weapon * const new_weapon,
                               const std::shared_ptr<WeaponAugmentsInstance>& new_augs,
                               const std::shared_ptr<WeaponUpgradesInstance>& new_upgs) noexcept
    : weapon   (new_weapon)
    , augments (new_augs)
    , upgrades (new_upgs)
{
}


WeaponContribution WeaponInstance::calculate_contribution() const {
    WeaponAugmentsContribution ac = this->augments->calculate_contribution();
    WeaponUpgradesContribution uc = this->upgrades->calculate_contribution();

    WeaponContribution ret = {
        this->weapon->true_raw + ac.added_raw + uc.added_raw,
        this->weapon->affinity + ac.added_aff + uc.added_aff,

        this->weapon->elestat_visibility,
        this->weapon->elestat_type,
        ((double)this->weapon->elestat_value) + ac.added_elestat_value,

        this->weapon->deco_slots,
        this->weapon->skill,
        uc.set_bonus,

        uc.sharpness_gauge_override,
        this->weapon->is_constant_sharpness,

        ac.health_regen_active,
    };

    // Now, we finish off any bits we have remaining.
    if (ac.extra_deco_slot_size) {
        ret.deco_slots.emplace_back(ac.extra_deco_slot_size);
    }
    if (uc.extra_deco_slot_size) {
        ret.deco_slots.emplace_back(uc.extra_deco_slot_size);
    }

    // And we have to sort it.
    std::sort(ret.deco_slots.begin(), ret.deco_slots.end(), std::greater<unsigned int>());

    return ret;
}


std::string WeaponInstance::get_humanreadable() const {
    return this->weapon->name
           + "\n\n" + this->upgrades->get_humanreadable()
           + "\n\n" + this->augments->get_humanreadable();
}


} // namespace

