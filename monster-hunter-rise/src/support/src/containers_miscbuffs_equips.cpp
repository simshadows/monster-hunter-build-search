/*
 * File: containers_miscbuffs_equips.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <stdexcept>
#include <algorithm>

#include "../support.h"
#include "../../utils/utils.h"


namespace MHRBuildSearch
{


MiscBuffsEquips::MiscBuffsEquips(ContainerType&& new_data)
    : data                (std::move(new_data))
    , added_raw           {0}
    , base_raw_multiplier {1.0}
{
    std::unordered_set<std::string> present_uniqueness_tags;
    for (const MiscBuff * const e : this->data) {
        for (const std::string& ee : e->uniqueness_tags) {
            if (Utils::set_has_key(present_uniqueness_tags, ee)) {
                throw std::runtime_error("Buff '" + e->id + "' cannot be applied due "
                                         "to mutual exclusion with another buff.");
            }
            present_uniqueness_tags.emplace(ee);
        }

        this->added_raw += e->added_raw;
        this->base_raw_multiplier *= e->base_raw_multiplier;
    }
}


unsigned int MiscBuffsEquips::get_added_raw() const {
    return this->added_raw;
}


double MiscBuffsEquips::get_base_raw_multiplier() const {
    return this->base_raw_multiplier;
}


std::string MiscBuffsEquips::get_humanreadable() const {
    if (this->data.size()) {
        std::string ret;
        bool first = true;
        for (const MiscBuff * const miscbuff : this->data) {
            if (!first) ret += "\n";
            ret += miscbuff->name;
            if (miscbuff->added_raw) {
                ret += " [Added Raw: " + std::to_string(miscbuff->added_raw) + "]";
            }
            if (miscbuff->base_raw_multiplier != 1.0) {
                ret += " [Base Raw Multiplier: " + std::to_string(miscbuff->base_raw_multiplier) + "]";
            }
            first = false;
        }
        return ret;
    } else {
        return "(no miscellaneous buffs)";
    }
}


} // namespace

