/*
 * File: weapon_upgrades.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include "core.h"
#include "../utils/utils.h"

namespace MHWIBuildSearch
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

    WeaponUpgradesContribution calculate_contribution() const {
        return {0, 0, 0, weapon->maximum_sharpness, ""};
    }

    std::string get_humanreadable() const {
        return "Weapon upgrades:\n  (This weapon cannot be upgraded.)";
    }

    void add_upgrade(WeaponUpgrade) {
        throw std::logic_error("Attempted to upgrade a weapon that cannot be upgraded.");
    }
};


/****************************************************************************************
 * IBCustomWeaponUpgrades
 ***************************************************************************************/


static const std::unordered_map<unsigned int, unsigned int> ib_custom_attack = {
    {0, 1},
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
    {5, 1},
    {6, 2},
};

static const std::unordered_map<unsigned int, int> ib_custom_affinity = {
    {0, 1},
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
    //{5, 0}, // Not Present
    {6, 3},
};


class IBCustomWeaponUpgrades : public WeaponUpgradesInstance {
    const Weapon * const weapon;
    std::vector<WeaponUpgrade> upgrades;
public:
    IBCustomWeaponUpgrades(const Weapon * const new_weapon) noexcept
        : weapon (new_weapon)
        , upgrades {}
    {
    }

    WeaponUpgradesContribution calculate_contribution() const {
        WeaponUpgradesContribution ret = {0, 0, 0, weapon->maximum_sharpness, ""};

        for (std::size_t i = 0; i < this->upgrades.size(); ++i) {
            switch (this->upgrades[i]) {
                case WeaponUpgrade::ib_cust_attack:
                    ret.added_raw += ib_custom_attack.at(i);
                    break;
                case WeaponUpgrade::ib_cust_affinity:
                    ret.added_aff += ib_custom_affinity.at(i);
                    break;
                default:
                    throw std::logic_error("Attempted to use an unsupported upgrade.");
            }
        }
        
        return ret;
    }

    std::string get_humanreadable() const {
        std::string ret = "Weapon upgrades:";
        if (this->upgrades.size() == 0) {
            ret += "\n  (no upgrades)";
        } else {
            for (std::size_t i = 0; i < this->upgrades.size(); ++i) {
                switch (this->upgrades[i]) {
                    case WeaponUpgrade::ib_cust_attack:
                        ret += "\n  Attack ";
                        break;
                    case WeaponUpgrade::ib_cust_affinity:
                        ret += "\n  Affinity ";
                        break;
                    default:
                        throw std::logic_error("Attempted to use an unsupported upgrade.");
                }
                ret += std::to_string(i + 1);
            }
        }
        return ret;
    }

    void add_upgrade(WeaponUpgrade upgrade) {
        // First, we test if it's valid.
        const std::size_t next_index = this->upgrades.size();
        switch (upgrade) {
            case WeaponUpgrade::ib_cust_attack:
                if (!Utils::map_has_key(ib_custom_attack, next_index)) {
                    throw InvalidChange("Attack upgrade cannot be applied at this time..");
                }
                break;
            case WeaponUpgrade::ib_cust_affinity:
                if (!Utils::map_has_key(ib_custom_affinity, next_index)) {
                    throw InvalidChange("Affinity upgrade cannot be applied at this time..");
                }
                break;
            default:
                throw InvalidChange("Attempted to apply an unsupported weapon upgrade.");
        }

        // Now, we add it!
        this->upgrades.emplace_back(upgrade);
    }
};


/****************************************************************************************
 * IBSafiAwakenings
 ***************************************************************************************/


static const std::unordered_set<WeaponUpgrade> ib_safi_lvl6_awakenings = {
    WeaponUpgrade::ib_safi_attack_6,
    WeaponUpgrade::ib_safi_affinity_6,
    WeaponUpgrade::ib_safi_sharpness_6,
    WeaponUpgrade::ib_safi_deco_slot_6,
};

static const std::unordered_set<WeaponUpgrade> ib_safi_deco_slot_awakenings = {
    WeaponUpgrade::ib_safi_deco_slot_1,
    WeaponUpgrade::ib_safi_deco_slot_2,
    WeaponUpgrade::ib_safi_deco_slot_3,
    WeaponUpgrade::ib_safi_deco_slot_6,
};

static const std::unordered_map<WeaponUpgrade, std::string> ib_safi_set_bonus_id_map = {
    {WeaponUpgrade::ib_safi_sb_ancient_divinity       , "ANCIENT_DIVINITY"       },
    {WeaponUpgrade::ib_safi_sb_anjanath_dominance     , "ANJANATH_DOMINANCE"     },
    {WeaponUpgrade::ib_safi_sb_barioth_hidden_art     , "BARIOTH_HIDDEN_ART"     },
    {WeaponUpgrade::ib_safi_sb_bazelgeuse_ambition    , "BAZELGEUSE_AMBITION"    },
    {WeaponUpgrade::ib_safi_sb_brachydios_essence     , "BRACHYDIOS_ESSENCE"     },
    {WeaponUpgrade::ib_safi_sb_deviljho_essence       , "DEVILJHO_ESSENCE"       },
    {WeaponUpgrade::ib_safi_sb_diablos_ambition       , "DIABLOS_AMBITION"       },
    {WeaponUpgrade::ib_safi_sb_glavenus_essence       , "GLAVENUS_ESSENCE"       },
    {WeaponUpgrade::ib_safi_sb_gold_rathian_essence   , "GOLD_RATHIAN_ESSENCE"   },
    {WeaponUpgrade::ib_safi_sb_kirin_divinity         , "KIRIN_DIVINITY"         },
    {WeaponUpgrade::ib_safi_sb_kushala_daora_flight   , "KUSHALA_DAORA_FLIGHT"   },
    {WeaponUpgrade::ib_safi_sb_legiana_ambition       , "LEGIANA_AMBITION"       },
    {WeaponUpgrade::ib_safi_sb_lunastra_essence       , "LUNASTRA_ESSENCE"       },
    {WeaponUpgrade::ib_safi_sb_namielle_divinity      , "NAMIELLE_DIVINITY"      },
    {WeaponUpgrade::ib_safi_sb_nargacuga_essence      , "NARGACUGA_ESSENCE"      },
    {WeaponUpgrade::ib_safi_sb_nergigante_ambition    , "NERGIGANTE_AMBITION"    },
    {WeaponUpgrade::ib_safi_sb_odogaron_essence       , "ODOGARON_ESSENCE"       },
    {WeaponUpgrade::ib_safi_sb_rajangs_rage           , "RAJANGS_RAGE"           },
    {WeaponUpgrade::ib_safi_sb_rathalos_essence       , "RATHALOS_ESSENCE"       },
    {WeaponUpgrade::ib_safi_sb_rathian_essence        , "RATHIAN_ESSENCE"        },
    {WeaponUpgrade::ib_safi_sb_shara_ishvalda_divinity, "SHARA_ISHVALDA_DIVINITY"},
    {WeaponUpgrade::ib_safi_sb_silver_rathalos_essence, "SILVER_RATHALOS_ESSENCE"},
    {WeaponUpgrade::ib_safi_sb_teostra_technique      , "TEOSTRA_TECHNIQUE"      },
    {WeaponUpgrade::ib_safi_sb_tigrex_essence         , "TIGREX_ESSENCE"         },
    {WeaponUpgrade::ib_safi_sb_uragaan_ambition       , "URAGAAN_AMBITION"       },
    {WeaponUpgrade::ib_safi_sb_vaal_soulvein          , "VAAL_SOULVEIN"          },
    {WeaponUpgrade::ib_safi_sb_velkhana_divinity      , "VELKHANA_DIVINITY"      },
    {WeaponUpgrade::ib_safi_sb_zinogre_essence        , "ZINOGRE_ESSENCE"        },
    {WeaponUpgrade::ib_safi_sb_zorah_magdaros_essence , "ZORAH_MAGDAROS_ESSENCE" },
};

static const std::unordered_map<WeaponUpgrade, std::string> ib_safi_supported_upgrades = {
    {WeaponUpgrade::ib_safi_attack_4   , "Attack Increase IV"},
    {WeaponUpgrade::ib_safi_attack_5   , "Attack Increase V"},
    {WeaponUpgrade::ib_safi_attack_6   , "Attack Increase VI"},
    {WeaponUpgrade::ib_safi_affinity_4 , "Affinity Increase IV"},
    {WeaponUpgrade::ib_safi_affinity_5 , "Affinity Increase V"},
    {WeaponUpgrade::ib_safi_affinity_6 , "Affinity Increase VI"},
    {WeaponUpgrade::ib_safi_sharpness_4, "Sharpness Increase IV"},
    {WeaponUpgrade::ib_safi_sharpness_5, "Sharpness Increase V"},
    {WeaponUpgrade::ib_safi_sharpness_6, "Sharpness Increase VI"},
    {WeaponUpgrade::ib_safi_deco_slot_1, "Slot Upgrade I"},
    {WeaponUpgrade::ib_safi_deco_slot_2, "Slot Upgrade II"},
    {WeaponUpgrade::ib_safi_deco_slot_3, "Slot Upgrade III"},
    {WeaponUpgrade::ib_safi_deco_slot_6, "Slot Upgrade VI"},

    {WeaponUpgrade::ib_safi_sb_ancient_divinity       , "Ancient Divinity"       },
    {WeaponUpgrade::ib_safi_sb_anjanath_dominance     , "Anjanath Dominance"     },
    {WeaponUpgrade::ib_safi_sb_barioth_hidden_art     , "Barioth Hidden Art"     },
    {WeaponUpgrade::ib_safi_sb_bazelgeuse_ambition    , "Bazelgeuse Ambition"    },
    {WeaponUpgrade::ib_safi_sb_brachydios_essence     , "Brachydios Essence"     },
    {WeaponUpgrade::ib_safi_sb_deviljho_essence       , "Deviljho Essence"       },
    {WeaponUpgrade::ib_safi_sb_diablos_ambition       , "Diablos Ambition"       },
    {WeaponUpgrade::ib_safi_sb_glavenus_essence       , "Glavenus Essence"       },
    {WeaponUpgrade::ib_safi_sb_gold_rathian_essence   , "Gold Rathian Essence"   },
    {WeaponUpgrade::ib_safi_sb_kirin_divinity         , "Kirin Divinity"         },
    {WeaponUpgrade::ib_safi_sb_kushala_daora_flight   , "Kushala Daora Flight"   },
    {WeaponUpgrade::ib_safi_sb_legiana_ambition       , "Legiana Ambition"       },
    {WeaponUpgrade::ib_safi_sb_lunastra_essence       , "Lunastra Essence"       },
    {WeaponUpgrade::ib_safi_sb_namielle_divinity      , "Namielle Divinity"      },
    {WeaponUpgrade::ib_safi_sb_nargacuga_essence      , "Nargacuga Essence"      },
    {WeaponUpgrade::ib_safi_sb_nergigante_ambition    , "Nergigante Ambition"    },
    {WeaponUpgrade::ib_safi_sb_odogaron_essence       , "Odogaron Essence"       },
    {WeaponUpgrade::ib_safi_sb_rajangs_rage           , "Rajang's Rage"          },
    {WeaponUpgrade::ib_safi_sb_rathalos_essence       , "Rathalos Essence"       },
    {WeaponUpgrade::ib_safi_sb_rathian_essence        , "Rathian Essence"        },
    {WeaponUpgrade::ib_safi_sb_shara_ishvalda_divinity, "Shara Ishvalda Divinity"},
    {WeaponUpgrade::ib_safi_sb_silver_rathalos_essence, "Silver Rathalos Essence"},
    {WeaponUpgrade::ib_safi_sb_teostra_technique      , "Teostra Technique"      },
    {WeaponUpgrade::ib_safi_sb_tigrex_essence         , "Tigrex Essence"         },
    {WeaponUpgrade::ib_safi_sb_uragaan_ambition       , "Uragaan Ambition"       },
    {WeaponUpgrade::ib_safi_sb_vaal_soulvein          , "Vaal Soulvein"          },
    {WeaponUpgrade::ib_safi_sb_velkhana_divinity      , "Velkhana Divinity"      },
    {WeaponUpgrade::ib_safi_sb_zinogre_essence        , "Zinogre Essence"        },
    {WeaponUpgrade::ib_safi_sb_zorah_magdaros_essence , "Zorah Magdaros Essence" },
};


static constexpr std::size_t k_MAX_AWAKENINGS = 5;

static constexpr unsigned int k_BASE_SHARPNESS_RED    = 100;
static constexpr unsigned int k_BASE_SHARPNESS_ORANGE = 50;
static constexpr unsigned int k_BASE_SHARPNESS_YELLOW = 50;
static constexpr unsigned int k_BASE_SHARPNESS_GREEN  = 50;
static constexpr unsigned int k_BASE_SHARPNESS_BLUE   = 50;
static constexpr unsigned int k_BASE_SHARPNESS_WHITE  = 90;
//static constexpr unsigned int k_BASE_SHARPNESS_PURPLE = 0;
static constexpr unsigned int k_MAX_WHITE_SHARPNESS_BEFORE_PURPLE = 120;


class IBSafiAwakenings : public WeaponUpgradesInstance {
    //const Weapon * const weapon;
    std::vector<WeaponUpgrade> awakenings;
public:
    IBSafiAwakenings(const Weapon * const new_weapon) noexcept
        //: weapon     (new_weapon)
        : awakenings {}
    {
        (void)new_weapon;
    }

    WeaponUpgradesContribution calculate_contribution() const {
        unsigned int added_raw = 0;
        int          added_aff = 0;
        unsigned int extra_deco_slot_size = 0;
        std::string  set_bonus_id = "";

        unsigned int white_sharpness = k_BASE_SHARPNESS_WHITE;

        for (const WeaponUpgrade& e : this->awakenings) {
            if (Utils::map_has_key(ib_safi_set_bonus_id_map, e)) {
                assert(set_bonus_id == "");
                set_bonus_id = ib_safi_set_bonus_id_map.at(e);
            } else {
                switch (e) {
                    case WeaponUpgrade::ib_safi_attack_4:    added_raw += 7;           break;
                    case WeaponUpgrade::ib_safi_attack_5:    added_raw += 9;           break;
                    case WeaponUpgrade::ib_safi_attack_6:    added_raw += 14;          break;
                    case WeaponUpgrade::ib_safi_affinity_4:  added_aff += 8;           break;
                    case WeaponUpgrade::ib_safi_affinity_5:  added_aff += 10;          break;
                    case WeaponUpgrade::ib_safi_affinity_6:  added_aff += 15;          break;
                    case WeaponUpgrade::ib_safi_sharpness_4: white_sharpness += 40;    break;
                    case WeaponUpgrade::ib_safi_sharpness_5: white_sharpness += 50;    break;
                    case WeaponUpgrade::ib_safi_sharpness_6: white_sharpness += 70;    break;
                    case WeaponUpgrade::ib_safi_deco_slot_1: extra_deco_slot_size = 1; break;
                    case WeaponUpgrade::ib_safi_deco_slot_2: extra_deco_slot_size = 2; break;
                    case WeaponUpgrade::ib_safi_deco_slot_3: extra_deco_slot_size = 3; break;
                    case WeaponUpgrade::ib_safi_deco_slot_6: extra_deco_slot_size = 4; break;
                    default:
                        throw std::logic_error("Attempted to use an unsupported upgrade.");
                }
            }
        }

        unsigned int purple_sharpness = 0;
        
        if (white_sharpness > k_MAX_WHITE_SHARPNESS_BEFORE_PURPLE) {
            purple_sharpness = white_sharpness - k_MAX_WHITE_SHARPNESS_BEFORE_PURPLE;
            white_sharpness = k_MAX_WHITE_SHARPNESS_BEFORE_PURPLE;
        }

        return {
            added_raw,
            added_aff,
            extra_deco_slot_size,
            SharpnessGauge(k_BASE_SHARPNESS_RED,
                           k_BASE_SHARPNESS_ORANGE,
                           k_BASE_SHARPNESS_YELLOW,
                           k_BASE_SHARPNESS_GREEN,
                           k_BASE_SHARPNESS_BLUE,
                           white_sharpness,
                           purple_sharpness),
            std::move(set_bonus_id),
        };
    }

    std::string get_humanreadable() const {
        std::string ret = "Weapon Awakenings:";
        if (this->awakenings.size() == 0) {
            ret += "\n  (no awakenings)";
        } else {
            for (const WeaponUpgrade& e : this->awakenings) {
                ret += "\n  " + ib_safi_supported_upgrades.at(e);
            }
        }
        return ret;
    }

    void add_upgrade(WeaponUpgrade awakening) {
        if (!Utils::map_has_key(ib_safi_supported_upgrades, awakening)) {
            throw InvalidChange("Attempted to apply an unsupported awakening.");
        }

        std::vector<WeaponUpgrade> new_awakenings = this->awakenings;
        new_awakenings.emplace_back(awakening);
        
        if (awakenings_is_valid(new_awakenings)) {
            this->awakenings = new_awakenings;
        } else {
            throw InvalidChange("Cannot apply this awakening at this time.");
        }
    }
private:
    static bool awakenings_is_valid(const std::vector<WeaponUpgrade>& test_awakenings) {
        if (test_awakenings.size() > k_MAX_AWAKENINGS) return false;

        bool has_lvl6      = false;
        bool has_slot      = false;
        bool has_set_bonus = false;
        for (const WeaponUpgrade& e : test_awakenings) {
            if (Utils::set_has_key(ib_safi_lvl6_awakenings, e)) {
                if (has_lvl6) return false;
                has_lvl6 = true;
            } else if (Utils::set_has_key(ib_safi_deco_slot_awakenings, e)) {
                if (has_slot) return false;
                has_slot = true;
            } else if (Utils::map_has_key(ib_safi_set_bonus_id_map, e)) {
                if (has_set_bonus) return false;
                has_set_bonus = true;
            }
        }
        return true;
    }
};


std::unique_ptr<WeaponUpgradesInstance> WeaponUpgradesInstance::get_instance(const Weapon * const weapon) {
    switch (weapon->upgrade_scheme) {
        case WeaponUpgradeScheme::none:            return std::make_unique<NoWeaponUpgrades>(weapon);
        case WeaponUpgradeScheme::iceborne_custom: return std::make_unique<IBCustomWeaponUpgrades>(weapon);
        case WeaponUpgradeScheme::iceborne_safi:   return std::make_unique<IBSafiAwakenings>(weapon);
        default:
            throw std::runtime_error("This weapon's augmentation type is unsupported.");
    }
}


} // namespace

