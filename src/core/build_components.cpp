/*
 * File: build_components.cpp
 * Author: <contact@simshadows.com>
 */

#include "core.h"

namespace MHWIBuildSearch
{

/****************************************************************************************
 * Basic Build Components: Skill and SetBonus
 ***************************************************************************************/


Skill::Skill(std::string new_id,
             std::string new_name,
             unsigned int new_normal_limit,
             unsigned int new_secret_limit,
             unsigned int new_states) noexcept
    : id           (new_id)
    , name         (new_name)
    , normal_limit (new_normal_limit)
    , secret_limit (new_secret_limit)
    , states       (new_states)
{
}


SetBonus::SetBonus(const std::string&& new_id,
                   const std::string&& new_name,
                   const std::vector<std::pair<unsigned int, const Skill*>>&& new_stages) noexcept
    : id     (std::move(new_id    ))
    , name   (std::move(new_name  ))
    , stages (std::move(new_stages))
{
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


// Nothing necessary...


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

