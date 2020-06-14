/*
 * File: core.h
 * Author: <contact@simshadows.com>
 */

#ifndef MHWIBS_CORE_H
#define MHWIBS_CORE_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_set>

namespace MHWIBuildSearch
{


constexpr unsigned int k_MIN_RARITY = 1;
constexpr unsigned int k_MAX_RARITY = 12;

enum class Tier {
    low_rank,
    high_rank,
    master_rank,
};

enum class EleStatType {
    none,

    // Elements
    fire,
    water,
    thunder,
    ice,
    dragon,

    // Statuses
    poison,
    paralysis,
    sleep,
    blast,

    //// We won't include these in this enum.
    //ko,
    //exhaust,
    //mount,
};


bool elestattype_is_element(EleStatType); // True if element, False if status. Undefined if EleStatType::none.
std::string elestattype_to_str(EleStatType);


class InvalidChange : public std::exception {
    const char * const msg;
public:
    InvalidChange(const char * const new_msg) noexcept
        : msg (new_msg)
    {
    }
    const char* what() const noexcept {
        return msg;
    }
};


/****************************************************************************************
 * SharpnessGauge
 ***************************************************************************************/


enum class SharpnessLevel {
    red,
    orange,
    yellow,
    green,
    blue,
    white,
    purple,
};

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
    static double sharpness_level_to_raw_sharpness_modifier(SharpnessLevel);

    // Constructs a new SharpnessGauge that is the result of applying handicraft to
    // another SharpnessGauge.
    SharpnessGauge apply_handicraft(unsigned int handicraft_lvl) const noexcept;

    double get_raw_sharpness_modifier() const; // Uses the full sharpness gauge.
    double get_raw_sharpness_modifier(unsigned int handicraft_lvl) const;

    double get_elemental_sharpness_modifier() const; // Uses the full sharpness gauge.

    SharpnessLevel get_sharpness_level() const;

    std::string get_humanreadable() const;

    static bool left_has_eq_or_more_hits(const SharpnessGauge& lhs, const SharpnessGauge& rhs) noexcept;

protected:

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
    const std::size_t  nid;           // Unique numerical ID used internally for algorithmic purposes.
                                      // NIDs must be guaranteed to be "as low as possible", allowing for
                                      // use in jump tables.

    const std::string  id;            // The official UPPER_SNAKE_CASE unique identifier of the skill.

    const char*        name;          // Actual skill name, as it appears in-game.
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
};


struct SetBonus {
    const std::string id;
    const std::string name;
    const std::vector<std::pair<unsigned int, const Skill*>> stages; // tuple format: (number of stages, skill)
};


/****************************************************************************************
 * Basic Build Components: Decoration
 ***************************************************************************************/


constexpr unsigned int k_MIN_DECO_SIZE = 1;
constexpr unsigned int k_MAX_DECO_SIZE = 4;


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

enum class EleStatVisibility {
    none,   // Always elementless.
    open,   // Always has an element.
    hidden, // Elementless, but has a "hidden element" that can be activated
            // under certain conditions (usually the Free Element skill).
};

enum class WeaponAugmentationScheme {
    none,
    //base_game, // Currently unused.
    iceborne,
};

enum class WeaponUpgradeScheme {
    none,
    iceborne_custom,
    iceborne_safi,
};

WeaponClass upper_snake_case_to_weaponclass(std::string s);


struct Weapon {
    const std::string    id; // The "UPPER_SNAKE_CASE" identifier of the weapon.

    const WeaponClass    weapon_class;

    const std::string    name; // Actual name, as it appears in-game.
    const unsigned int   rarity;
    const unsigned int   true_raw; // Must be converted from its bloated raw.
    const int            affinity;

    const EleStatVisibility elestat_visibility;
    const EleStatType       elestat_type;
    const unsigned int      elestat_value;

    const std::vector<unsigned int> deco_slots;

    const Skill* const   skill; // nullptr if no skill.

    const WeaponAugmentationScheme augmentation_scheme;
    const WeaponUpgradeScheme      upgrade_scheme;

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
Tier armour_variant_to_tier(ArmourVariant);


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
 * Basic Build Components: MiscBuff
 ***************************************************************************************/


struct MiscBuff {
    const std::string id;
    const std::string name;
    //const std::string buff_class; // The class field in the database is purely for human use.
    const unsigned int added_raw;
    const double base_raw_multiplier;
    const std::unordered_set<std::string> uniqueness_tags;
};


/****************************************************************************************
 * WeaponAugmentsInstance
 ***************************************************************************************/


enum class WeaponAugment {
    augment_lvl, // Technically not an augment.

    attack_increase,
    affinity_increase,
    //defense_increase, // Not yet supported.
    slot_upgrade,
    health_regen,
    element_status_effect_up,
};


struct WeaponAugmentsContribution {
    unsigned int added_raw            {0};
    int          added_aff            {0};
    double       added_elestat_value  {0};
    unsigned int extra_deco_slot_size {0};
    bool         health_regen_active  {false};
};


class WeaponAugmentsInstance {
public:
    static std::shared_ptr<WeaponAugmentsInstance> get_instance(const Weapon*);
    static std::vector<std::shared_ptr<WeaponAugmentsInstance>> generate_maximized_instances(const Weapon*);

    // Access
    virtual WeaponAugmentsContribution calculate_contribution() const = 0;
    virtual std::string get_humanreadable() const = 0;

    // Modification
    // (These methods will throw InvalidChange if the requested change is rejected.
    // If InvalidChange is throw, then the class state will be unchanged.)
    virtual void set_augment(WeaponAugment augment, unsigned int lvl) = 0;

    virtual ~WeaponAugmentsInstance() {}
};


/****************************************************************************************
 * WeaponUpgradesInstance
 ***************************************************************************************/


enum class WeaponUpgrade {
    // IB Custom Augments
    ib_cust_attack,
    ib_cust_affinity,
    ib_cust_element_status,
    //ib_cust_defense,        // Not yet supported.
    //ib_cust_sharpness,      // Not yet supported.

    // IB Safi
    ib_safi_attack_4,
    ib_safi_attack_5,
    ib_safi_attack_6,
    ib_safi_affinity_4,
    ib_safi_affinity_5,
    ib_safi_affinity_6,
    ib_safi_sharpness_4,
    ib_safi_sharpness_5,
    ib_safi_sharpness_6,
    //ib_safi_deco_slot_1, // This upgrade doesn't exist.
    //ib_safi_deco_slot_2, // This upgrade doesn't exist.
    ib_safi_deco_slot_1,
    ib_safi_deco_slot_2,
    ib_safi_deco_slot_3,
    ib_safi_deco_slot_6,
    ib_safi_sb_ancient_divinity,
    ib_safi_sb_anjanath_dominance,
    ib_safi_sb_barioth_hidden_art,
    ib_safi_sb_bazelgeuse_ambition,
    ib_safi_sb_brachydios_essence,
    ib_safi_sb_deviljho_essence,
    ib_safi_sb_diablos_ambition,
    ib_safi_sb_glavenus_essence,
    ib_safi_sb_gold_rathian_essence,
    ib_safi_sb_kirin_divinity,
    ib_safi_sb_kushala_daora_flight,
    ib_safi_sb_legiana_ambition,
    ib_safi_sb_lunastra_essence,
    ib_safi_sb_namielle_divinity,
    ib_safi_sb_nargacuga_essence,
    ib_safi_sb_nergigante_ambition,
    ib_safi_sb_odogaron_essence,
    ib_safi_sb_rajangs_rage,
    ib_safi_sb_rathalos_essence,
    ib_safi_sb_rathian_essence,
    ib_safi_sb_shara_ishvalda_divinity,
    ib_safi_sb_silver_rathalos_essence,
    ib_safi_sb_teostra_technique,
    ib_safi_sb_tigrex_essence,
    ib_safi_sb_uragaan_ambition,
    ib_safi_sb_vaal_soulvein,
    ib_safi_sb_velkhana_divinity,
    ib_safi_sb_zinogre_essence,
    ib_safi_sb_zorah_magdaros_essence,
    // Many other Safi awakenings are not yet supported.
};


struct WeaponUpgradesContribution {
    unsigned int    added_raw;
    int             added_aff;
    double          added_elestat_value;
    unsigned int    extra_deco_slot_size;
    SharpnessGauge  sharpness_gauge_override;
    const SetBonus* set_bonus;
};


class WeaponUpgradesInstance {
public:
    static std::shared_ptr<WeaponUpgradesInstance> get_instance(const Weapon*);
    static std::vector<std::shared_ptr<WeaponUpgradesInstance>> generate_maximized_instances(const Weapon*);

    // Access
    virtual WeaponUpgradesContribution calculate_contribution() const = 0;
    virtual std::string get_humanreadable() const = 0;

    // Modification
    // (These methods will throw InvalidChange if the requested change is rejected.
    // If InvalidChange is throw, then the class state will be unchanged.)
    virtual void add_upgrade(WeaponUpgrade upgrade) = 0;

    virtual ~WeaponUpgradesInstance() {}
};


} // namespace

#endif // MHWIBS_CORE_H

