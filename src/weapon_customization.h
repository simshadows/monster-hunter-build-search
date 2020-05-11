/*
 * File: weapon_customization.h
 * Author: <contact@simshadows.com>
 */

#ifndef WEAPON_CUSTOMIZATION_H
#define WEAPON_CUSTOMIZATION_H

#include <memory>
#include <unordered_map>

#include "database/database.h"

namespace MHWIBuildSearch
{


/****************************************************************************************
 * WeaponAugmentsInstance
 ***************************************************************************************/


struct WeaponAugmentsContribution {
    unsigned int added_raw;
    int          added_aff;
    unsigned int extra_deco_slot_size;
    bool         health_regen_active;
};


class WeaponAugmentsInstance {
public:
    static std::unique_ptr<WeaponAugmentsInstance> get_instance(const Database::Weapon&);

    virtual WeaponAugmentsContribution calculate_contribution() const = 0;
    
    virtual std::string get_humanreadable() const = 0;

    virtual ~WeaponAugmentsInstance() {}
};


} // namespace

#endif // WEAPON_CUSTOMIZATION_H

