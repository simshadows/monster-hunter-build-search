/*
 * File: weapon_augments.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <unordered_set>
#include <unordered_map>

#include "../core.h"
#include "../../utils/utils.h"

namespace MHWIBuildSearch
{


static constexpr unsigned int k_IB_MIN_RARITY = 10;
static constexpr unsigned int k_IB_MAX_RARITY = 12;

//static constexpr unsigned int k_IB_MAX_AUGMENT_LVL = 3;

static const std::unordered_set<WeaponAugment> ib_supported_augments = {
    WeaponAugment::augment_lvl,

    WeaponAugment::attack_increase,
    WeaponAugment::affinity_increase,
    //WeaponAugment::defense_increase, // Not yet supported.
    WeaponAugment::slot_upgrade,
    WeaponAugment::health_regen,
    //WeaponAugment::element_status_effect_up, // Not yet supported.
};

//                                                             level: 0,  1,  2,  3,  4
static const std::array<unsigned int, 5> ib_attack_aug_added_raw   = {0,  5, 10, 15, 20};
static const std::array<unsigned int, 5> ib_affinity_aug_added_aff = {0, 10, 15, 20, 25};


class NoWeaponAugments : public WeaponAugmentsInstance {
public:
    NoWeaponAugments() noexcept = default;

    WeaponAugmentsContribution calculate_contribution() const {
        return {0, 0, 0, false};
    }
    
    std::string get_humanreadable() const {
        return "Weapon augments:\n  (This weapon cannot be augmented.)";
    }

    void set_augment(const WeaponAugment, const unsigned int) {
        throw std::logic_error("Attempted to augment a weapon that cannot be augmented.");
    }
};


class IBWeaponAugments : public WeaponAugmentsInstance {

    using AugmentLvlMap = std::unordered_map<WeaponAugment, unsigned int>;

    const unsigned int rarity;
    unsigned int augment_lvl;
    AugmentLvlMap augments; // Augment levels
public:
    IBWeaponAugments(const Weapon * const weapon) noexcept
        : rarity      (weapon->rarity)
        , augment_lvl (0)
        , augments    ()
    {
        assert(this->rarity >= k_IB_MIN_RARITY); // Not allowed any other rarity.
        assert(this->rarity <= k_IB_MAX_RARITY);
        (void)k_IB_MIN_RARITY;
        (void)k_IB_MAX_RARITY;
    }

    WeaponAugmentsContribution calculate_contribution() const {
        WeaponAugmentsContribution ret;
        for (const auto& e : this->augments) {
            const unsigned int lvl = e.second;
            switch (e.first) {
                case WeaponAugment::attack_increase:
                    ret.added_raw = ib_attack_aug_added_raw[lvl];
                    break;
                case WeaponAugment::affinity_increase:
                    ret.added_aff = ib_affinity_aug_added_aff[lvl];
                    break;
                //case WeaponAugment::defense_increase: // Not yet supported.
                case WeaponAugment::slot_upgrade:
                    assert(lvl >= k_MIN_DECO_SIZE);
                    assert(lvl <= k_MAX_DECO_SIZE);
                    ret.extra_deco_slot_size = lvl;
                    break;
                case WeaponAugment::health_regen:
                    ret.health_regen_active = lvl; // Will be "true" as long as lvl is not zero.
                    break;
                //case WeaponAugment::element_status_effect_up: // Not yet supported.
                default:
                    throw std::logic_error("Invalid augment for IBWeaponAugments.");
            }
        }
        return ret;
    }
    
    std::string get_humanreadable() const {
        std::string ret = "Weapon Augments:";
        if ((this->augment_lvl == 0) && (this->augments.size() == 0)) {
            ret += "\n  (no augments)";
        } else {
            if (this->augment_lvl > 0) {
                ret += "\n  Extra Slots " + Utils::to_capital_roman_numerals(this->augment_lvl);
            }
            for (const auto& e : this->augments) {
                const WeaponAugment augment = e.first;
                const unsigned int lvl = e.second;
                ret += "\n  " + get_augment_humanreadable(augment) + " "
                       + Utils::to_capital_roman_numerals(lvl);
            }
        }
        return ret;
    }

    void set_augment(const WeaponAugment augment, const unsigned int lvl) {
        if (!Utils::set_has_key(ib_supported_augments, augment)) {
            throw InvalidChange("Attempted to apply an unsupported weapon augment.");
        }

        if (augment == WeaponAugment::augment_lvl) {
            const unsigned int old_consumption = calculate_slot_consumption_from_map(this->augments);
            const unsigned int new_limit = calculate_slot_limit(this->rarity, lvl);
            if (new_limit < old_consumption) throw InvalidChange("New slot level cannot support existing augments.");
            this->augment_lvl = lvl;
        } else {
            AugmentLvlMap new_augments = this->augments;
            new_augments[augment] = lvl;
            const unsigned int old_limit = calculate_slot_limit(this->rarity, this->augment_lvl);
            const unsigned int new_consumption = calculate_slot_consumption_from_map(new_augments);
            if (new_consumption > old_limit) throw InvalidChange("New augments cannot be supported by current slot limit.");
            this->augments = std::move(new_augments);
        }
    }

private:

    unsigned int get_augment_lvl(const WeaponAugment augment) const {
        if (Utils::map_has_key(this->augments, augment)) {
            return this->augments.at(augment);
        } else {
            return 0;
        }
    }

    // TODO: Figure out a better way to do this?
    static unsigned int calculate_slot_limit(const unsigned int test_rarity,
                                             const unsigned int test_augment_lvl) {
        switch (test_rarity) {
            case 10:
                switch (test_augment_lvl) {
                    case 0: return 5;
                    case 1: return 7;
                    case 2: return 9;
                    case 3: return 10;
                    default: throw std::logic_error("Invalid slot level.");
                }
            case 11:
                switch (test_augment_lvl) {
                    case 0: return 4;
                    case 1: return 5;
                    case 2: return 6;
                    case 3: return 8;
                    default: throw std::logic_error("Invalid slot level.");
                }
            case 12:
                switch (test_augment_lvl) {
                    case 0: return 3;
                    case 1: return 4;
                    case 2: return 5;
                    case 3: return 6;
                    default: throw std::logic_error("Invalid slot level.");
                }
            default:
                throw std::logic_error("Invalid weapon rarity.");
        }
    }

    static unsigned int calculate_slot_consumption_from_map(const AugmentLvlMap& test_augments) {
        unsigned int ret = 0;
        for (const auto& e : test_augments) {
            ret += get_augment_accumulated_consumption(e.first, e.second);
        }
        return ret;
    }

    // TODO: Figure out a better way to do this?
    static unsigned int get_augment_accumulated_consumption(const WeaponAugment v,
                                                            const unsigned int lvl) {
        switch (v) {
            case WeaponAugment::attack_increase:
                switch (lvl) {
                    case 0: return 0;
                    case 1: return 3;
                    case 2: return 3 + 2;
                    case 3: return 3 + 2 + 2;
                    case 4: return 3 + 2 + 2 + 2;
                    default: throw std::logic_error("Invalid Attack Increase level.");
                }
            case WeaponAugment::affinity_increase:
                switch (lvl) {
                    case 0: return 0;
                    case 1: return 2;
                    case 2: return 2 + 2;
                    case 3: return 2 + 2 + 2;
                    case 4: return 2 + 2 + 2 + 2;
                    default: throw std::logic_error("Invalid Affinity Increase level.");
                }
            //case WeaponAugment::defense_increase: // Not yet supported.
            //    switch (lvl) {
            //        case 0: return 0;
            //        case 1: return 1;
            //        case 2: return 1 + 1;
            //        case 3: return 1 + 1 + 1;
            //        case 4: return 1 + 1 + 1 + 2;
            //        default: throw std::logic_error("Invalid Defense Increase level.");
            //    }
            case WeaponAugment::slot_upgrade:
                switch (lvl) {
                    case 0: return 0;
                    case 1: return 3;
                    case 2: return 3 + 3;
                    case 3: return 3 + 3 + 1;
                    case 4: return 3 + 3 + 1 + 1;
                    default: throw std::logic_error("Invalid Slot Upgrade level.");
                }
            case WeaponAugment::health_regen:
                switch (lvl) {
                    case 0: return 0;
                    case 1: return 3;
                    case 2: return 3 + 2;
                    case 3: return 3 + 2 + 2;
                    case 4: return 3 + 2 + 2 + 2;
                    default: throw std::logic_error("Invalid Health Regen level.");
                }
            //case WeaponAugment::element_status_effect_up: // Not yet supported.
            //    switch (lvl) {
            //        case 0: return 0;
            //        case 1: return 1;
            //        case 2: return 1 + 2;
            //        case 3: return 1 + 2 + 2;
            //        case 4: return 1 + 2 + 2 + 2;
            //        default: throw std::logic_error("Invalid Element/Status Effect Up level.");
            //    }
            default:
                throw std::logic_error("Invalid augment for IBWeaponAugments.");
        }
    }

    static std::string get_augment_humanreadable(const WeaponAugment v) {
        switch (v) {
            case WeaponAugment::attack_increase:            return "Attack Increase";
            case WeaponAugment::affinity_increase:          return "Affinity Increase";
            //case WeaponAugment::defense_increase:         return "Defense Increase"; // Not yet supported.
            case WeaponAugment::slot_upgrade:               return "Slot Upgrade";
            case WeaponAugment::health_regen:               return "Health Regen";
            //case WeaponAugment::element_status_effect_up: return "Element/Status Effect Up"; // Not yet supported.
            default:
                throw std::logic_error("Invalid augment for IBWeaponAugments.");
        }
    }
};


std::unique_ptr<WeaponAugmentsInstance> WeaponAugmentsInstance::get_instance(const Weapon * const weapon) {
    switch (weapon->augmentation_scheme) {
        case WeaponAugmentationScheme::none:     return std::make_unique<NoWeaponAugments>();
        case WeaponAugmentationScheme::iceborne: return std::make_unique<IBWeaponAugments>(weapon);
        default:
            throw std::runtime_error("This weapon's augmentation type is unsupported.");
    }
}


} // namespace

