/*
 * File: weapon_upgrades.cpp
 * Author: <contact@simshadows.com>
 */

#include <stdexcept>
#include <assert.h>

#include "../core.h"
#include "../../database/database_skills.h"
#include "../../utils/utils.h"

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
 * IBCustomWeaponUpgrades
 ***************************************************************************************/


static constexpr unsigned int k_IBC_MAX_UPGRADES = 7;

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

static const std::unordered_map<unsigned int, int> ib_custom_elestat_value = {
    {0, 1},
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
    //{5, 0}, // Not Present
    //{6, 0}, // Not Present
};

static const std::array<WeaponUpgrade, 3> ibc_supported_upgrades_v = {
    WeaponUpgrade::ib_cust_attack,
    WeaponUpgrade::ib_cust_affinity,
    WeaponUpgrade::ib_cust_element_status,
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

    static std::vector<std::shared_ptr<WeaponUpgradesInstance>> generate_maximized_instances(const Weapon * const new_weapon) {

        std::vector<std::shared_ptr<IBCustomWeaponUpgrades>> ret = {
            std::make_shared<IBCustomWeaponUpgrades>(new_weapon) // Seeded with an empty upgrade instance
        };

        // ughhhh
        for (std::size_t i = 0; i < k_IBC_MAX_UPGRADES; ++i) {
            std::vector<std::shared_ptr<IBCustomWeaponUpgrades>> new_ret;
            for (const WeaponUpgrade u : ibc_supported_upgrades_v) {
                for (const std::shared_ptr<IBCustomWeaponUpgrades>& old_inst : ret) {
                    IBCustomWeaponUpgrades new_inst = *old_inst;
                    bool valid_new_inst = true;
                    try {
                        new_inst.add_upgrade(u);
                    } catch (const InvalidChange&) {
                        valid_new_inst = false;
                    }
                    if (valid_new_inst) new_ret.push_back(std::make_shared<IBCustomWeaponUpgrades>(std::move(new_inst)));
                }
            }
            ret = std::move(new_ret);
        }

        // This is absolutely disgusting.
        std::vector<std::shared_ptr<WeaponUpgradesInstance>> final_ret;
        for (std::shared_ptr<IBCustomWeaponUpgrades>& e : ret) {
            final_ret.push_back(std::make_shared<IBCustomWeaponUpgrades>(std::move(*e)));
        }

        return final_ret;
    }

    WeaponUpgradesContribution calculate_contribution() const {
        WeaponUpgradesContribution ret = {0, 0, 0, 0, weapon->maximum_sharpness, nullptr};

        for (std::size_t i = 0; i < this->upgrades.size(); ++i) {
            switch (this->upgrades[i]) {
                case WeaponUpgrade::ib_cust_attack:
                    ret.added_raw += ib_custom_attack.at(i);
                    break;
                case WeaponUpgrade::ib_cust_affinity:
                    ret.added_aff += ib_custom_affinity.at(i);
                    break;
                case WeaponUpgrade::ib_cust_element_status:
                    ret.added_elestat_value += ib_custom_elestat_value.at(i);
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
            ret += "\n    (no upgrades)";
        } else {
            for (std::size_t i = 0; i < this->upgrades.size(); ++i) {
                switch (this->upgrades[i]) {
                    case WeaponUpgrade::ib_cust_attack:
                        ret += "\n    Attack ";
                        break;
                    case WeaponUpgrade::ib_cust_affinity:
                        ret += "\n    Affinity ";
                        break;
                    case WeaponUpgrade::ib_cust_element_status:
                        ret += "\n    Element/Status ";
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
            case WeaponUpgrade::ib_cust_element_status:
                if (!Utils::map_has_key(ib_custom_elestat_value, next_index)) {
                    throw InvalidChange("Element/status upgrade cannot be applied at this time..");
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


static const std::vector<WeaponUpgrade> ib_safi_nondeco_lvl5_awakenings = {
    WeaponUpgrade::ib_safi_attack_5,
    WeaponUpgrade::ib_safi_affinity_5,
    WeaponUpgrade::ib_safi_element_5,
    WeaponUpgrade::ib_safi_status_5,
    WeaponUpgrade::ib_safi_sharpness_5,
};

static const std::unordered_set<WeaponUpgrade> ib_safi_lvl6_awakenings = {
    WeaponUpgrade::ib_safi_attack_6,
    WeaponUpgrade::ib_safi_affinity_6,
    WeaponUpgrade::ib_safi_element_6,
    WeaponUpgrade::ib_safi_status_6,
    WeaponUpgrade::ib_safi_sharpness_6,
    WeaponUpgrade::ib_safi_deco_slot_6,
};

static const std::unordered_set<WeaponUpgrade> ib_safi_deco_slot_awakenings = {
    WeaponUpgrade::ib_safi_deco_slot_1,
    WeaponUpgrade::ib_safi_deco_slot_2,
    WeaponUpgrade::ib_safi_deco_slot_3,
    WeaponUpgrade::ib_safi_deco_slot_6,
};

static const std::unordered_map<WeaponUpgrade, const SetBonus*> ib_safi_set_bonus_map = {
    {WeaponUpgrade::ib_safi_sb_ancient_divinity       , &SkillsDatabase::g_setbonus_ancient_divinity       },
    {WeaponUpgrade::ib_safi_sb_anjanath_dominance     , &SkillsDatabase::g_setbonus_anjanath_dominance     },
    {WeaponUpgrade::ib_safi_sb_barioth_hidden_art     , &SkillsDatabase::g_setbonus_barioth_hidden_art     },
    {WeaponUpgrade::ib_safi_sb_bazelgeuse_ambition    , &SkillsDatabase::g_setbonus_bazelgeuse_ambition    },
    {WeaponUpgrade::ib_safi_sb_brachydios_essence     , &SkillsDatabase::g_setbonus_brachydios_essence     },
    {WeaponUpgrade::ib_safi_sb_deviljho_essence       , &SkillsDatabase::g_setbonus_deviljho_essence       },
    {WeaponUpgrade::ib_safi_sb_diablos_ambition       , &SkillsDatabase::g_setbonus_diablos_ambition       },
    {WeaponUpgrade::ib_safi_sb_glavenus_essence       , &SkillsDatabase::g_setbonus_glavenus_essence       },
    {WeaponUpgrade::ib_safi_sb_gold_rathian_essence   , &SkillsDatabase::g_setbonus_gold_rathian_essence   },
    {WeaponUpgrade::ib_safi_sb_kirin_divinity         , &SkillsDatabase::g_setbonus_kirin_divinity         },
    {WeaponUpgrade::ib_safi_sb_kushala_daora_flight   , &SkillsDatabase::g_setbonus_kushala_daora_flight   },
    {WeaponUpgrade::ib_safi_sb_legiana_ambition       , &SkillsDatabase::g_setbonus_legiana_ambition       },
    {WeaponUpgrade::ib_safi_sb_lunastra_essence       , &SkillsDatabase::g_setbonus_lunastra_essence       },
    {WeaponUpgrade::ib_safi_sb_namielle_divinity      , &SkillsDatabase::g_setbonus_namielle_divinity      },
    {WeaponUpgrade::ib_safi_sb_nargacuga_essence      , &SkillsDatabase::g_setbonus_nargacuga_essence      },
    {WeaponUpgrade::ib_safi_sb_nergigante_ambition    , &SkillsDatabase::g_setbonus_nergigante_ambition    },
    {WeaponUpgrade::ib_safi_sb_odogaron_essence       , &SkillsDatabase::g_setbonus_odogaron_essence       },
    {WeaponUpgrade::ib_safi_sb_rajangs_rage           , &SkillsDatabase::g_setbonus_rajangs_rage           },
    {WeaponUpgrade::ib_safi_sb_rathalos_essence       , &SkillsDatabase::g_setbonus_rathalos_essence       },
    {WeaponUpgrade::ib_safi_sb_rathian_essence        , &SkillsDatabase::g_setbonus_rathian_essence        },
    {WeaponUpgrade::ib_safi_sb_shara_ishvalda_divinity, &SkillsDatabase::g_setbonus_shara_ishvalda_divinity},
    {WeaponUpgrade::ib_safi_sb_silver_rathalos_essence, &SkillsDatabase::g_setbonus_silver_rathalos_essence},
    {WeaponUpgrade::ib_safi_sb_teostra_technique      , &SkillsDatabase::g_setbonus_teostra_technique      },
    {WeaponUpgrade::ib_safi_sb_tigrex_essence         , &SkillsDatabase::g_setbonus_tigrex_essence         },
    {WeaponUpgrade::ib_safi_sb_uragaan_ambition       , &SkillsDatabase::g_setbonus_uragaan_ambition       },
    {WeaponUpgrade::ib_safi_sb_vaal_soulvein          , &SkillsDatabase::g_setbonus_vaal_soulvein          },
    {WeaponUpgrade::ib_safi_sb_velkhana_divinity      , &SkillsDatabase::g_setbonus_velkhana_divinity      },
    {WeaponUpgrade::ib_safi_sb_zinogre_essence        , &SkillsDatabase::g_setbonus_zinogre_essence        },
    {WeaponUpgrade::ib_safi_sb_zorah_magdaros_essence , &SkillsDatabase::g_setbonus_zorah_magdaros_essence },
};

static const std::unordered_map<WeaponUpgrade, std::string> ib_safi_supported_upgrades = {
    {WeaponUpgrade::ib_safi_attack_4   , "Attack Increase IV"   },
    {WeaponUpgrade::ib_safi_attack_5   , "Attack Increase V"    },
    {WeaponUpgrade::ib_safi_attack_6   , "Attack Increase VI"   },
    {WeaponUpgrade::ib_safi_affinity_4 , "Affinity Increase IV" },
    {WeaponUpgrade::ib_safi_affinity_5 , "Affinity Increase V"  },
    {WeaponUpgrade::ib_safi_affinity_6 , "Affinity Increase VI" },
    {WeaponUpgrade::ib_safi_element_4  , "Element Up IV"        },
    {WeaponUpgrade::ib_safi_element_5  , "Element Up V"         },
    {WeaponUpgrade::ib_safi_element_6  , "Element Up VI"        },
    {WeaponUpgrade::ib_safi_status_4   , "Status Effect Up IV"  },
    {WeaponUpgrade::ib_safi_status_5   , "Status Effect Up V"   },
    {WeaponUpgrade::ib_safi_status_6   , "Status Effect Up VI"  },
    {WeaponUpgrade::ib_safi_sharpness_4, "Sharpness Increase IV"},
    {WeaponUpgrade::ib_safi_sharpness_5, "Sharpness Increase V" },
    {WeaponUpgrade::ib_safi_sharpness_6, "Sharpness Increase VI"},
    {WeaponUpgrade::ib_safi_deco_slot_1, "Slot Upgrade I"       },
    {WeaponUpgrade::ib_safi_deco_slot_2, "Slot Upgrade II"      },
    {WeaponUpgrade::ib_safi_deco_slot_3, "Slot Upgrade III"     },
    {WeaponUpgrade::ib_safi_deco_slot_6, "Slot Upgrade VI"      },

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

static const std::unordered_set<WeaponUpgrade> ib_safi_unsupported_by_elemental_weapons = {
    WeaponUpgrade::ib_safi_status_4,
    WeaponUpgrade::ib_safi_status_5,
    WeaponUpgrade::ib_safi_status_6,
};

static const std::unordered_set<WeaponUpgrade> ib_safi_unsupported_by_status_weapons = {
    WeaponUpgrade::ib_safi_element_4,
    WeaponUpgrade::ib_safi_element_5,
    WeaponUpgrade::ib_safi_element_6,
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
    const EleStatType weapon_elestat_type;
public:
    IBSafiAwakenings(const Weapon * const new_weapon) noexcept
        //: weapon     (new_weapon)
        : awakenings {}
        , weapon_elestat_type (new_weapon->elestat_type)
    {
        assert(this->weapon_elestat_type != EleStatType::none);
    }

    static std::vector<std::shared_ptr<WeaponUpgradesInstance>> generate_maximized_instances(const Weapon * const new_weapon) {
        std::vector<WeaponUpgrade> only_lvl6;
        only_lvl6.insert(only_lvl6.end(), ib_safi_lvl6_awakenings.begin(), ib_safi_lvl6_awakenings.end());

        std::vector<WeaponUpgrade> lvl5_or_deco = ib_safi_nondeco_lvl5_awakenings;
        lvl5_or_deco.insert(lvl5_or_deco.end(), ib_safi_deco_slot_awakenings.begin(), ib_safi_deco_slot_awakenings.end());

        std::vector<WeaponUpgrade> lvl5_or_sb = ib_safi_nondeco_lvl5_awakenings;
        for (const auto& e : ib_safi_set_bonus_map) {
            lvl5_or_sb.push_back(e.first);
        }

        // so inefficient
        std::array<std::vector<WeaponUpgrade>, k_MAX_AWAKENINGS> choices = {
            only_lvl6,
            lvl5_or_deco,
            lvl5_or_sb,
            ib_safi_nondeco_lvl5_awakenings,
            ib_safi_nondeco_lvl5_awakenings,
        };

        std::vector<std::shared_ptr<IBSafiAwakenings>> ret = {
            std::make_shared<IBSafiAwakenings>(new_weapon) // Seeded with an empty upgrade instance
        };

        // so bad
        for (const auto& choice : choices) {
            std::vector<std::shared_ptr<IBSafiAwakenings>> new_ret;
            for (const WeaponUpgrade u : choice) {
                for (const std::shared_ptr<IBSafiAwakenings>& old_inst : ret) {
                    IBSafiAwakenings new_inst = *old_inst;
                    bool valid_new_inst = true;
                    try {
                        new_inst.add_upgrade(u);
                    } catch (const InvalidChange&) {
                        valid_new_inst = false;
                    }
                    if (valid_new_inst) new_ret.push_back(std::make_shared<IBSafiAwakenings>(std::move(new_inst)));
                }
            }
            ret = std::move(new_ret);
        }

        // This is absolutely disgusting.
        std::vector<std::shared_ptr<WeaponUpgradesInstance>> final_ret;
        for (std::shared_ptr<IBSafiAwakenings>& e : ret) {
            final_ret.push_back(std::make_shared<IBSafiAwakenings>(std::move(*e)));
        }

        return final_ret;
    }

    WeaponUpgradesContribution calculate_contribution() const {
        unsigned int    added_raw = 0;
        int             added_aff = 0;
        double          added_elestat_value = 0;
        unsigned int    extra_deco_slot_size = 0;
        const SetBonus* set_bonus = nullptr;

        unsigned int white_sharpness = k_BASE_SHARPNESS_WHITE;

        const auto req_ele = [&](){
            assert(this->weapon_elestat_type != EleStatType::none);
            assert(elestattype_is_element(this->weapon_elestat_type));
        };
        const auto req_stat = [&](){
            assert(this->weapon_elestat_type != EleStatType::none);
            assert(!elestattype_is_element(this->weapon_elestat_type));
        };

        for (const WeaponUpgrade& e : this->awakenings) {
            if (Utils::map_has_key(ib_safi_set_bonus_map, e)) {
                assert(!set_bonus);
                set_bonus = ib_safi_set_bonus_map.at(e);
            } else {
                switch (e) {
                    case WeaponUpgrade::ib_safi_attack_4:                added_raw += 7;            break;
                    case WeaponUpgrade::ib_safi_attack_5:                added_raw += 9;            break;
                    case WeaponUpgrade::ib_safi_attack_6:                added_raw += 14;           break;
                    case WeaponUpgrade::ib_safi_affinity_4:              added_aff += 8;            break;
                    case WeaponUpgrade::ib_safi_affinity_5:              added_aff += 10;           break;
                    case WeaponUpgrade::ib_safi_affinity_6:              added_aff += 15;           break;
                    case WeaponUpgrade::ib_safi_element_4:   req_ele();  added_elestat_value += 9;  break;
                    case WeaponUpgrade::ib_safi_element_5:   req_ele();  added_elestat_value += 12; break;
                    case WeaponUpgrade::ib_safi_element_6:   req_ele();  added_elestat_value += 15; break;
                    case WeaponUpgrade::ib_safi_status_4:    req_stat(); added_elestat_value += 4;  break;
                    case WeaponUpgrade::ib_safi_status_5:    req_stat(); added_elestat_value += 5;  break;
                    case WeaponUpgrade::ib_safi_status_6:    req_stat(); added_elestat_value += 7;  break;
                    case WeaponUpgrade::ib_safi_sharpness_4:             white_sharpness += 40;     break;
                    case WeaponUpgrade::ib_safi_sharpness_5:             white_sharpness += 50;     break;
                    case WeaponUpgrade::ib_safi_sharpness_6:             white_sharpness += 70;     break;
                    case WeaponUpgrade::ib_safi_deco_slot_1:             extra_deco_slot_size = 1;  break;
                    case WeaponUpgrade::ib_safi_deco_slot_2:             extra_deco_slot_size = 2;  break;
                    case WeaponUpgrade::ib_safi_deco_slot_3:             extra_deco_slot_size = 3;  break;
                    case WeaponUpgrade::ib_safi_deco_slot_6:             extra_deco_slot_size = 4;  break;
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
            added_elestat_value,
            extra_deco_slot_size,
            SharpnessGauge(k_BASE_SHARPNESS_RED,
                           k_BASE_SHARPNESS_ORANGE,
                           k_BASE_SHARPNESS_YELLOW,
                           k_BASE_SHARPNESS_GREEN,
                           k_BASE_SHARPNESS_BLUE,
                           white_sharpness,
                           purple_sharpness),
            set_bonus,
        };
    }

    std::string get_humanreadable() const {
        std::string ret = "Weapon Awakenings:";
        if (this->awakenings.size() == 0) {
            ret += "\n    (no awakenings)";
        } else {
            for (const WeaponUpgrade& e : this->awakenings) {
                ret += "\n    " + ib_safi_supported_upgrades.at(e);
            }
        }
        return ret;
    }

    void add_upgrade(WeaponUpgrade awakening) {
        if (!Utils::map_has_key(ib_safi_supported_upgrades, awakening)) {
            throw InvalidChange("Attempted to apply an unsupported awakening.");
        }
        assert(this->weapon_elestat_type != EleStatType::none);
        if (elestattype_is_element(this->weapon_elestat_type)
                        && Utils::set_has_key(ib_safi_unsupported_by_elemental_weapons, awakening) ) {
            throw InvalidChange("Attempted to apply an awakening that is not supported by elemental weapons.");
        } else if (Utils::set_has_key(ib_safi_unsupported_by_status_weapons, awakening)) {
            throw InvalidChange("Attempted to apply an awakening that is not supported by status weapons.");
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
            } else if (Utils::map_has_key(ib_safi_set_bonus_map, e)) {
                if (has_set_bonus) return false;
                has_set_bonus = true;
            }
        }
        return true;
    }
};


std::shared_ptr<WeaponUpgradesInstance> WeaponUpgradesInstance::get_instance(const Weapon * const weapon) {
    switch (weapon->upgrade_scheme) {
        case WeaponUpgradeScheme::none:            return std::make_shared<NoWeaponUpgrades>(weapon);
        case WeaponUpgradeScheme::iceborne_custom: return std::make_shared<IBCustomWeaponUpgrades>(weapon);
        case WeaponUpgradeScheme::iceborne_safi:   return std::make_shared<IBSafiAwakenings>(weapon);
        default:
            throw std::runtime_error("This weapon's upgrade type is unsupported.");
    }
}


std::vector<std::shared_ptr<WeaponUpgradesInstance>> WeaponUpgradesInstance::generate_maximized_instances(const Weapon * const weapon) {
    switch (weapon->upgrade_scheme) {
        case WeaponUpgradeScheme::none:            return NoWeaponUpgrades::generate_maximized_instances(weapon);
        case WeaponUpgradeScheme::iceborne_custom: return IBCustomWeaponUpgrades::generate_maximized_instances(weapon);
        case WeaponUpgradeScheme::iceborne_safi:   return IBSafiAwakenings::generate_maximized_instances(weapon);
        default:
            throw std::runtime_error("This weapon's upgrade type is unsupported.");
    }
}


} // namespace

