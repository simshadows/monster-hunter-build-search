/*
 * File: core.h
 * Author: <contact@simshadows.com>
 */

#ifndef MHWIBS_CORE_H
#define MHWIBS_CORE_H

#include <unordered_map>

#include "../database/database.h"

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

#endif // MHWIBS_CORE_H

