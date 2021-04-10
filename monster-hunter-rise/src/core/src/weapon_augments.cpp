/*
 * File: weapon_augments.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <unordered_set>
#include <unordered_map>

#include "../core.h"
#include "../../utils/utils.h"
#include "../../utils/utils_strings.h"
#include "../../utils/counter.h"

namespace MHRBuildSearch
{


static constexpr unsigned int k_IB_MIN_RARITY = 10;
static constexpr unsigned int k_IB_MAX_RARITY = 12;

static constexpr unsigned int k_IB_MAX_AUGMENT_LVL = 3; // The maximum augment level of the entire weapon
static constexpr unsigned int k_IB_AUGMENT_MAX_LVL = 4; // The maximum level of each individual augment
// TODO: Let's make this a less ambiguous. Let's use different terminology?

static const std::unordered_set<WeaponAugment> ib_supported_augments = {

    //WeaponAugment::augment_lvl, // Supported, but not really an augment, so we're not including it here.

    WeaponAugment::attack_increase,
    WeaponAugment::affinity_increase,
    //WeaponAugment::defense_increase, // Not yet supported.
    WeaponAugment::slot_upgrade,
    WeaponAugment::health_regen,
    WeaponAugment::element_status_effect_up,
};
static const std::vector<WeaponAugment> ib_supported_augments_v (ib_supported_augments.begin(),
                                                                 ib_supported_augments.end() );

//                                                                             level: 0,   1,   2,    3,    4
static const std::array<unsigned int, 5> ib_attack_aug_added_raw                   = {0,   5,  10,   15,   20};
static const std::array<unsigned int, 5> ib_affinity_aug_added_aff                 = {0,  10,  15,   20,   25};
static const std::array<double, 5>       ib_elestat_aug_added_elestat_value_gs     = {0, 3.6, 7.2, 10.8, 14.4};
static const std::array<double, 5>       ib_elestat_aug_added_elestat_value_hhgl   = {0, 3.3, 6.6,  9.9, 13.2};
static const std::array<double, 5>       ib_elestat_aug_added_elestat_value_others = {0,   3,   6,    9,   12};


class NoWeaponAugments : public WeaponAugmentsInstance {
public:
    NoWeaponAugments() noexcept = default;

    static std::vector<std::shared_ptr<WeaponAugmentsInstance>> generate_maximized_instances() {
        return {std::make_shared<NoWeaponAugments>()};
    }

    WeaponAugmentsContribution calculate_contribution() const {
        return {0, 0, 0, false};
    }
    
    std::string get_humanreadable() const {
        return "Weapon augments:\n    (This weapon cannot be augmented.)";
    }

    void set_augment(const WeaponAugment, const unsigned int) {
        throw std::logic_error("Attempted to augment a weapon that cannot be augmented.");
    }
};


class IBWeaponAugments : public WeaponAugmentsInstance {

    using AugmentLvls = Utils::Counter<WeaponAugment>;

    const unsigned int rarity;
    const WeaponClass weapon_class;
    unsigned int augment_lvl;
    AugmentLvls augments; // Augment levels
public:
    IBWeaponAugments(const Weapon * const weapon) noexcept
        : rarity       (weapon->rarity)
        , weapon_class (weapon->weapon_class)
        , augment_lvl  (0)
        , augments     ()
    {
        assert(this->rarity >= k_IB_MIN_RARITY); // Not allowed any other rarity.
        assert(this->rarity <= k_IB_MAX_RARITY);
        (void)k_IB_MIN_RARITY;
        (void)k_IB_MAX_RARITY;
    }

    static std::vector<std::shared_ptr<WeaponAugmentsInstance>> generate_maximized_instances(const Weapon * const weapon) {
        IBWeaponAugments base (weapon);
        base.set_augment(WeaponAugment::augment_lvl, k_IB_MAX_AUGMENT_LVL);

        std::vector<AugmentLvls> maps = AugmentLvls::generate_power_set(ib_supported_augments_v, k_IB_AUGMENT_MAX_LVL);

        std::vector<std::shared_ptr<WeaponAugmentsInstance>> ret;
        for (const AugmentLvls& map : maps) {
            IBWeaponAugments new_augs = base;
            bool valid_new_augs = true;
            try {
                for (const auto& x : map) {
                    new_augs.set_augment(x.first, x.second);
                }
            } catch (const InvalidChange&) {
                valid_new_augs = false;
            }
            if (valid_new_augs) ret.push_back(std::make_shared<IBWeaponAugments>(std::move(new_augs)));
        }

        return ret;
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
                case WeaponAugment::element_status_effect_up: {
                        switch (this->weapon_class) {
                            case WeaponClass::greatsword:
                                ret.added_elestat_value += ib_elestat_aug_added_elestat_value_gs[lvl];
                                break;
                            case WeaponClass::hunting_horn:
                            case WeaponClass::gunlance:
                                ret.added_elestat_value += ib_elestat_aug_added_elestat_value_hhgl[lvl];
                                break;
                            case WeaponClass::longsword:
                            case WeaponClass::sword_and_shield:
                            case WeaponClass::dual_blades:
                            case WeaponClass::hammer:
                            case WeaponClass::lance:
                            case WeaponClass::switchaxe:
                            case WeaponClass::charge_blade:
                            case WeaponClass::insect_glaive:
                                ret.added_elestat_value += ib_elestat_aug_added_elestat_value_others[lvl];
                                break;
                            case WeaponClass::bow:
                                ret.added_elestat_value += ib_elestat_aug_added_elestat_value_others[lvl];
                                // TODO: Implement status coating increase.
                                break;
                            case WeaponClass::heavy_bowgun:
                            case WeaponClass::light_bowgun:
                                throw std::logic_error("Bowguns are currently unsupported.");
                            default:
                                throw std::logic_error("Invalid weapon class.");
                        }
                    } break;
                default:
                    throw std::logic_error("Invalid augment for IBWeaponAugments.");
            }
        }
        return ret;
    }
    
    std::string get_humanreadable() const {
        std::string ret = "Weapon Augments:";
        if ((this->augment_lvl == 0) && (this->augments.size() == 0)) {
            ret += "\n    (no augments)";
        } else {
            if (this->augment_lvl > 0) {
                ret += "\n    Extra Slots " + Utils::to_capital_roman_numerals(this->augment_lvl);
            }
            for (const auto& e : this->augments) {
                const WeaponAugment augment = e.first;
                const unsigned int lvl = e.second;
                ret += "\n    " + get_augment_humanreadable(augment) + " "
                       + Utils::to_capital_roman_numerals(lvl);
            }
        }
        return ret;
    }

    void set_augment(const WeaponAugment augment, const unsigned int lvl) {
        if (augment == WeaponAugment::augment_lvl) {
            const unsigned int old_consumption = calculate_slot_consumption_from_map(this->augments);
            const unsigned int new_limit = calculate_slot_limit(this->rarity, lvl);
            if (new_limit < old_consumption) throw InvalidChange("New slot level cannot support existing augments.");
            this->augment_lvl = lvl;
        } else {
            if (!Utils::set_has_key(ib_supported_augments, augment)) {
                throw InvalidChange("Attempted to apply an unsupported weapon augment.");
            }

            AugmentLvls new_augments = this->augments;
            new_augments.set(augment, lvl);
            const unsigned int old_limit = calculate_slot_limit(this->rarity, this->augment_lvl);
            const unsigned int new_consumption = calculate_slot_consumption_from_map(new_augments);
            if (new_consumption > old_limit) throw InvalidChange("New augments cannot be supported by current slot limit.");
            this->augments = std::move(new_augments);
        }
    }

private:

    unsigned int get_augment_lvl(const WeaponAugment augment) const {
        return this->augments.get(augment);
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

    static unsigned int calculate_slot_consumption_from_map(const AugmentLvls& test_augments) {
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
            case WeaponAugment::element_status_effect_up:
                switch (lvl) {
                    case 0: return 0;
                    case 1: return 1;
                    case 2: return 1 + 2;
                    case 3: return 1 + 2 + 2;
                    case 4: return 1 + 2 + 2 + 2;
                    default: throw std::logic_error("Invalid Element/Status Effect Up level.");
                }
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
            case WeaponAugment::element_status_effect_up:   return "Element/Status Effect Up";
            default:
                throw std::logic_error("Invalid augment for IBWeaponAugments.");
        }
    }
};


std::shared_ptr<WeaponAugmentsInstance> WeaponAugmentsInstance::get_instance(const Weapon * const weapon) {
    switch (weapon->augmentation_scheme) {
        case WeaponAugmentationScheme::none:     return std::make_shared<NoWeaponAugments>();
        case WeaponAugmentationScheme::iceborne: return std::make_shared<IBWeaponAugments>(weapon);
        default:
            throw std::runtime_error("This weapon's augmentation type is unsupported.");
    }
}


std::vector<std::shared_ptr<WeaponAugmentsInstance>> WeaponAugmentsInstance::generate_maximized_instances(const Weapon* weapon) {
    switch (weapon->augmentation_scheme) {
        case WeaponAugmentationScheme::none:     return NoWeaponAugments::generate_maximized_instances();
        case WeaponAugmentationScheme::iceborne: return IBWeaponAugments::generate_maximized_instances(weapon);
        default:
            throw std::runtime_error("This weapon's augmentation type is unsupported.");
    }
}


} // namespace

