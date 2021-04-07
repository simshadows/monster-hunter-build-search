/*
 * File: build_components.cpp
 * Author: <contact@simshadows.com>
 */

#include <stdexcept>
#include <unordered_map>

#include "../core.h"

namespace MHWIBuildSearch
{


bool elestattype_is_element(const EleStatType v) {
    switch (v) {
        //case EleStatType::none:
        //    throw std::logic_error("Cannot be none.");
        case EleStatType::fire:
        case EleStatType::water:
        case EleStatType::thunder:
        case EleStatType::ice:
        case EleStatType::dragon:
            return true;
        case EleStatType::poison:
        case EleStatType::sleep:
        case EleStatType::paralysis:
        case EleStatType::blast:
            return false;
        default:
            throw std::logic_error("Invalid EleStatType value.");
    }
}


std::string elestattype_to_str(const EleStatType v) {
    switch (v) {
        case EleStatType::none:      return "None";
        case EleStatType::fire:      return "Fire";
        case EleStatType::water:     return "Water";
        case EleStatType::thunder:   return "Thunder";
        case EleStatType::ice:       return "Ice";
        case EleStatType::dragon:    return "Dragon";
        case EleStatType::poison:    return "Poison";
        case EleStatType::sleep:     return "Sleep";
        case EleStatType::paralysis: return "Paralysis";
        case EleStatType::blast:     return "Blast";
        default:
            throw std::logic_error("Invalid EleStatType value.");
    }
}


/****************************************************************************************
 * Basic Build Components: Decoration
 ***************************************************************************************/

Decoration::Decoration(std::string&& new_id,
                       std::string&& new_name,
                       unsigned int new_slot_size,
                       std::vector<std::pair<const Skill*, unsigned int>>&& new_skills) noexcept
    : id        (std::move(new_id       ))
    , name      (std::move(new_name     ))
    , slot_size (std::move(new_slot_size))
    , skills    (std::move(new_skills   ))
{
}


/****************************************************************************************
 * Basic Build Components: Weapon
 ***************************************************************************************/


static const std::unordered_map<std::string, WeaponClass> upper_snake_case_to_weaponclass_map = {
    {"GREATSWORD"      , WeaponClass::greatsword      },
    {"LONGSWORD"       , WeaponClass::longsword       },
    {"SWORD_AND_SHIELD", WeaponClass::sword_and_shield},
    {"DUAL_BLADES"     , WeaponClass::dual_blades     },
    {"HAMMER"          , WeaponClass::hammer          },
    {"HUNTING_HORN"    , WeaponClass::hunting_horn    },
    {"LANCE"           , WeaponClass::lance           },
    {"GUNLANCE"        , WeaponClass::gunlance        },
    {"SWITCHAXE"       , WeaponClass::switchaxe       },
    {"CHARGE_BLADE"    , WeaponClass::charge_blade    },
    {"INSECT_GLAIVE"   , WeaponClass::insect_glaive   },
    {"BOW"             , WeaponClass::bow             },
    {"HEAVY_BOWGUN"    , WeaponClass::heavy_bowgun    },
    {"LIGHT_BOWGUN"    , WeaponClass::light_bowgun    },
};

WeaponClass upper_snake_case_to_weaponclass(std::string s) {
    return upper_snake_case_to_weaponclass_map.at(s);
}



/****************************************************************************************
 * Basic Build Components: Armour
 ***************************************************************************************/


std::string armour_variant_to_name(const ArmourVariant v) {
    switch (v) {
        case ArmourVariant::low_rank:               return "";
        case ArmourVariant::high_rank_alpha:        return "\u03b1";
        case ArmourVariant::high_rank_beta:         return "\u03b2";
        case ArmourVariant::high_rank_gamma:        return "\u03b3";
        case ArmourVariant::master_rank_alpha_plus: return "\u03b1+";
        case ArmourVariant::master_rank_beta_plus:  return "\u03b2+";
        case ArmourVariant::master_rank_gamma_plus: return "\u03b3+";
        default:
            throw std::runtime_error("Invalid armour variant.");
    }
}


Tier armour_variant_to_tier(const ArmourVariant v) {
    switch (v) {
        case ArmourVariant::low_rank:               return Tier::low_rank;
        case ArmourVariant::high_rank_alpha:        return Tier::high_rank;
        case ArmourVariant::high_rank_beta:         return Tier::high_rank;
        case ArmourVariant::high_rank_gamma:        return Tier::high_rank;
        case ArmourVariant::master_rank_alpha_plus: return Tier::master_rank;
        case ArmourVariant::master_rank_beta_plus:  return Tier::master_rank;
        case ArmourVariant::master_rank_gamma_plus: return Tier::master_rank;
        default:
            throw std::runtime_error("Invalid armour variant.");
    }
}


ArmourPiece::ArmourPiece(ArmourSlot                  new_slot,
                         ArmourVariant               new_variant,
                         std::vector<unsigned int>&& new_deco_slots,
                         std::vector<std::pair<const Skill*, unsigned int>>&& new_skills,
                         std::string&&               new_piece_name_postfix,
                         const ArmourSet*            new_set,
                         const SetBonus*             new_set_bonus) noexcept
    : slot               (std::move(new_slot              ))
    , variant            (std::move(new_variant           ))
    , deco_slots         (std::move(new_deco_slots        ))
    , skills             (std::move(new_skills            ))
    , piece_name_postfix (std::move(new_piece_name_postfix))
    , set                (std::move(new_set               ))
    , set_bonus          (std::move(new_set_bonus         ))
{
}


std::string ArmourPiece::get_full_name() const {
    const std::string& prefix = this->set->piece_name_prefix;
    const std::string& postfix = this->piece_name_postfix;
    const std::string variant_str = armour_variant_to_name(this->variant);
    if (variant_str.size() == 0) {
        return prefix + " " + postfix;
    } else {
        return prefix + " " + postfix + " " + variant_str;
    }
}


ArmourSet::ArmourSet(std::string&&   new_set_name,
                     Tier            new_tier,
                     std::string&&   new_piece_name_prefix,
                     unsigned int    new_rarity,
                     const SetBonus* new_set_bonus,
                     std::vector<std::shared_ptr<ArmourPiece>>&& new_pieces) noexcept
    : set_name          (std::move(new_set_name         ))
    , tier              (std::move(new_tier             ))
    , piece_name_prefix (std::move(new_piece_name_prefix))
    , rarity            (std::move(new_rarity           ))
    , set_bonus         (std::move(new_set_bonus        ))
    , pieces            (std::move(new_pieces           ))
{
}


/****************************************************************************************
 * Basic Build Components: Charm
 ***************************************************************************************/


Charm::Charm(std::string&&               new_id,
             std::string&&               new_name,
             unsigned int                new_max_charm_lvl,
             std::vector<const Skill*>&& new_skills) noexcept
    : id            (std::move(new_id           ))
    , name          (std::move(new_name         ))
    , max_charm_lvl (std::move(new_max_charm_lvl))
    , skills        (std::move(new_skills       ))
{
}


} // namespace

