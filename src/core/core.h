/*
 * File: core.h
 * Author: <contact@simshadows.com>
 */

#ifndef MHWIBS_CORE_H
#define MHWIBS_CORE_H

#include <memory>
#include <vector>

namespace MHWIBuildSearch
{


constexpr unsigned int k_MIN_RARITY = 1;
constexpr unsigned int k_MAX_RARITY = 12;

constexpr unsigned int k_MIN_DECO_SIZE = 1;
constexpr unsigned int k_MAX_DECO_SIZE = 4;

enum class Tier {
    low_rank,
    high_rank,
    master_rank,
};


/****************************************************************************************
 * SharpnessGauge
 ***************************************************************************************/


constexpr std::size_t k_SHARPNESS_LEVELS = 7;


class SharpnessGauge {
    std::array<unsigned int, k_SHARPNESS_LEVELS> hits;
public:

    SharpnessGauge(unsigned int r,
                   unsigned int o,
                   unsigned int y,
                   unsigned int g,
                   unsigned int b,
                   unsigned int w,
                   unsigned int p) noexcept;

    // Special constructor.
    // Throws an exception if the vector is of the wrong size.
    static SharpnessGauge from_vector(const std::vector<unsigned int>&);

    double get_raw_sharpness_modifier() const; // Uses the full sharpness gauge.
    double get_raw_sharpness_modifier(unsigned int handicraft_lvl) const;

    std::string get_humanreadable() const;

protected:

    // Constructs a new SharpnessGauge that is the result of applying handicraft to
    // another SharpnessGauge.
    //
    // MOVE TO PUBLIC AS NEEDED.
    SharpnessGauge apply_handicraft(unsigned int handicraft_lvl) const noexcept;

    // Minimal constructor.
    // Doesn't do more work than is necessary.
    //
    // TODO: Make sure that it really doesn't do too much work, like initializing all
    //       array elements to zeroes.
    // TODO: Will `SharpnessGauge() = default` implement a minimal constructor?
    //       That would be preferable over a more explicit contructor definition.
    SharpnessGauge() noexcept;
    
};


/****************************************************************************************
 * Basic Build Components: Skill and SetBonus
 ***************************************************************************************/


struct Skill {
    const std::string  id;            // The "UPPER_SNAKE_CASE" identifier of the skill.

    const std::string  name;          // Actual skill name, as it appears in-game.
    const unsigned int normal_limit;  // Maximum level without "secret" skills.
    const unsigned int secret_limit;  // Maximum level possible, usually only attainable with "secret" skills.
                                      // For programming convenience, normal_limit and secret_limit are equal
                                      // if no corresponding "secret" skill is available.

    const unsigned int states;        // Number of meaningful "states" that a skill has.
                                      // All skills have at least two states.
                                      // State 0 is always "off".
                                      // State 1 is always "on".
                                      // States beyond 1 are additional states that depend on the particular skill.
                                      // E.g. Weakness Exploit has three states depending on if you're hitting
                                      // a non-weakspot, a weakspot, or a tenderized weakspot.

    // The fields below are currently unused.

    //const std::string  tooltip;       // Tooltip for the skill, as it appears in-game.
    //const std::string  info;          // Additional information about a skill.

    //const std::string  previous_name; // Additional information about a skill.

    Skill(std::string  new_id,
          std::string  new_name,
          unsigned int new_normal_limit,
          unsigned int new_secret_limit,
          unsigned int new_states) noexcept;
};


struct SetBonus {
    const std::string id;
    const std::string name;
    const std::vector<std::tuple<unsigned int, const Skill*>> stages; // tuple format: (number of stages, skill)

    SetBonus(const std::string&&,
             const std::string&&,
             const std::vector<std::tuple<unsigned int, const Skill*>>&&) noexcept;
};


/****************************************************************************************
 * Basic Build Components: Decoration
 ***************************************************************************************/


struct Decoration {
    const std::string id;
    const std::string name;
    const unsigned int slot_size;
    const std::vector<std::pair<const Skill*, unsigned int>> skills; // Skill and level

    Decoration(std::string&& new_id,
               std::string&& new_name,
               unsigned int new_slot_size,
               std::vector<std::pair<const Skill*, unsigned int>>&& new_skills) noexcept;
};


/****************************************************************************************
 * Basic Build Components: Weapon
 ***************************************************************************************/


enum class WeaponClass {
    greatsword,
    longsword,
    sword_and_shield,
    dual_blades,
    hammer,
    hunting_horn,
    lance,
    gunlance,
    switchaxe,
    charge_blade,
    insect_glaive,
    bow,
    heavy_bowgun,
    light_bowgun,
};


enum class WeaponAugmentationScheme {
    none,
    //base_game, // Currently unused.
    iceborne,
};


struct Weapon {
    const std::string    id; // The "UPPER_SNAKE_CASE" identifier of the weapon.

    const WeaponClass    weapon_class;

    const std::string    name; // Actual name, as it appears in-game.
    const unsigned int   rarity;
    const unsigned int   true_raw; // Must be converted from its bloated raw.
    const int            affinity;

    const bool           is_raw; // Temporary implementation while I sort out how to do elemental calculations.

    const std::vector<unsigned int> deco_slots;

    const Skill* const   skill; // nullptr if no skill.

    const WeaponAugmentationScheme augmentation_scheme;
    const std::string              upgrade_scheme;

    const SharpnessGauge maximum_sharpness;
    const bool           is_constant_sharpness;
};


/****************************************************************************************
 * Basic Build Components: Armour
 ***************************************************************************************/


enum class ArmourSlot {
    head,
    chest,
    arms,
    waist,
    legs,
};


enum class ArmourVariant {
    low_rank,
    high_rank_alpha,
    high_rank_beta,
    high_rank_gamma,
    master_rank_alpha_plus,
    master_rank_beta_plus,
    master_rank_gamma_plus,
};
std::string armour_variant_to_name(ArmourVariant);


struct ArmourSet; // Quick declaration so we can use it in ArmourPiece.
struct ArmourPiece {
    const ArmourSlot    slot;
    const ArmourVariant variant;

    const std::vector<unsigned int> deco_slots;
    const std::vector<std::pair<const Skill*, unsigned int>> skills; // Skills and levels.

    const std::string      piece_name_postfix;

    const ArmourSet*       set;       // A pointer back to the armour set.
    const SetBonus * const set_bonus; // Supplied for convenience. Guaranteed to be the same as ArmourSet.

    ArmourPiece(ArmourSlot                  new_slot,
                ArmourVariant               new_variant,
                std::vector<unsigned int>&& new_deco_slots,
                std::vector<std::pair<const Skill*, unsigned int>>&& new_skills,
                std::string&&               new_piece_name_postfix,
                const ArmourSet*            new_set,
                const SetBonus*             new_set_bonus) noexcept;

    std::string get_full_name() const;
};


struct ArmourSet {
    // set_name and tier are required to uniquely identify an armour set.
    const std::string set_name;
    const Tier        tier;

    const std::string piece_name_prefix;

    const unsigned int     rarity;
    const SetBonus * const set_bonus; // Can be nullptr!

    std::vector<std::shared_ptr<ArmourPiece>> pieces; // CAN'T BE CONST

    ArmourSet(std::string&&   new_set_name,
              Tier            new_tier,
              std::string&&   new_piece_name_prefix,
              unsigned int    new_rarity,
              const SetBonus* new_set_bonus,
              std::vector<std::shared_ptr<ArmourPiece>>&& new_pieces) noexcept;
};


/****************************************************************************************
 * Basic Build Components: Charm
 ***************************************************************************************/


struct Charm {
    const std::string id;
    const std::string name;
    const unsigned int max_charm_lvl;
    // All skills will have levels equalling max_charm_level.
    // E.g. if max_charm_level is 3, then all skills in the vector will be level 3.
    const std::vector<const Skill*> skills;

    Charm(std::string&&               new_id,
          std::string&&               new_name,
          unsigned int                new_max_charm_lvl,
          std::vector<const Skill*>&& new_skills) noexcept;
};


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
    static std::unique_ptr<WeaponAugmentsInstance> get_instance(const Weapon&);

    virtual WeaponAugmentsContribution calculate_contribution() const = 0;
    
    virtual std::string get_humanreadable() const = 0;

    virtual ~WeaponAugmentsInstance() {}
};


} // namespace

#endif // MHWIBS_CORE_H

