/*
 * File: containers_deco_equips.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <algorithm>

#include "../support.h"
#include "../../utils/utils.h"


namespace MHWIBuildSearch
{


DecoEquips::DecoEquips() noexcept = default;


DecoEquips::DecoEquips(ContainerType&& new_data) noexcept
    : data (std::move(new_data))
{
}


bool DecoEquips::fits_in(const ArmourEquips& armour, const WeaponContribution& wc) const {
    // Get deco slots, sorted in descending order
    std::vector<unsigned int> deco_slots = armour.get_deco_slots();
    deco_slots.insert(deco_slots.end(), wc.deco_slots.begin(), wc.deco_slots.end());
    std::sort(deco_slots.begin(), deco_slots.end(), std::greater<unsigned int>());

    // Get decos, sorted by size in descending order
    std::vector<const Decoration*> decos = this->data;
    const auto cmp = [](const Decoration * const a, const Decoration * const b){return a->slot_size > b->slot_size;};
    std::sort(decos.begin(), decos.end(), cmp);

    auto p = deco_slots.begin();
    for (const Decoration* deco : decos) {
        if ((p == deco_slots.end()) || (*p < deco->slot_size)) return false;
        ++p;
    }
    return true;
}

void DecoEquips::add(const Decoration * const deco) {
    this->data.emplace_back(deco);
}

void DecoEquips::merge_in(const DecoEquips& obj) {
    this->data.insert(this->data.end(), obj.data.begin(), obj.data.end());
}

DecoEquips::IteratorType DecoEquips::begin() const {
    return this->data.begin();
}

DecoEquips::IteratorType DecoEquips::end() const {
    return this->data.end();
}

std::string DecoEquips::get_humanreadable() const {
    if (this->data.size()) {
        std::string ret;
        bool first = true;
        for (const Decoration * const deco : this->data) {
            if (!first) ret += "\n";
            ret += deco->name;
            first = false;
        }
        return ret;
    } else {
        return "(no decorations)";
    }
}


} // namespace

