/*
 * File: run_tests.cpp
 * Author: <contact@simshadows.com>
 *
 * Testing for MHWI Build Search.
 */

#define CATCH_CONFIG_MAIN
#include "../dependencies/catch-2-12-1/catch.hpp"

#include <unordered_map>

#include "../src/core/core.h"
#include "../src/database/database.h"
#include "../src/database/database_decorations.h"
#include "../src/database/database_miscbuffs.h"
#include "../src/database/database_skills.h"
#include "../src/support/support.h"
#include "../src/utils/utils.h"

namespace TestMHWIBuildSearch
{

using namespace MHWIBuildSearch;


// Convenience function to get a variant, purely for testing purposes.
static const std::unordered_map<std::string, ArmourVariant> vari = {
    {"L",  ArmourVariant::low_rank              },
    {"HA", ArmourVariant::high_rank_alpha       },
    {"HB", ArmourVariant::high_rank_beta        },
    {"HG", ArmourVariant::high_rank_gamma       },
    {"MA", ArmourVariant::master_rank_alpha_plus},
    {"MB", ArmourVariant::master_rank_beta_plus },
    {"MG", ArmourVariant::master_rank_gamma_plus},
};


// Convenience function, purely for testing purposes.
static ArmourEquips get_armour(std::string head_set,
                               std::string head_var,
                               std::string chest_set,
                               std::string chest_var,
                               std::string arms_set,
                               std::string arms_var,
                               std::string waist_set,
                               std::string waist_var,
                               std::string legs_set,
                               std::string legs_var,
                               std::string charm_id,
                               const Database& db) {
    ArmourEquips armour;
    if (head_set != "") {
        armour.add(db.armour.at(head_set,
                                armour_variant_to_tier(vari.at(head_var)),
                                vari.at(head_var),
                                ArmourSlot::head));
    }
    if (chest_set != "") {
        armour.add(db.armour.at(chest_set,
                                armour_variant_to_tier(vari.at(chest_var)),
                                vari.at(chest_var),
                                ArmourSlot::chest));
    }
    if (arms_set != "") {
        armour.add(db.armour.at(arms_set,
                                armour_variant_to_tier(vari.at(arms_var)),
                                vari.at(arms_var),
                                ArmourSlot::arms));
    }
    if (waist_set != "") {
        armour.add(db.armour.at(waist_set,
                                armour_variant_to_tier(vari.at(waist_var)),
                                vari.at(waist_var),
                                ArmourSlot::waist));
    }
    if (legs_set != "") {
        armour.add(db.armour.at(legs_set,
                                armour_variant_to_tier(vari.at(legs_var)),
                                vari.at(legs_var),
                                ArmourSlot::legs));
    }
    if (charm_id != "") {
        armour.add(db.charms.at(charm_id));
    }

    return armour;
}


// We set up a single global database, out of convenience.
static const Database db = Database::get_db();


TEST_CASE("Incrementally building up a greatsword Safi Shattersplitter build.") {

    std::unordered_map<const Skill*, unsigned int> min_levels = {
        {&SkillsDatabase::g_skill_agitator, 0},
        {&SkillsDatabase::g_skill_weakness_exploit, 0},
    };
    std::unordered_map<const Skill*, unsigned int> forced_states = {
        {&SkillsDatabase::g_skill_weakness_exploit, 1},
    };
    MiscBuffsEquips misc_buffs ({
        &MiscBuffsDatabase::get_miscbuff("POWERCHARM"),
        &MiscBuffsDatabase::get_miscbuff("POWERTALON"),
    });
    SkillSpec skill_spec(std::move(min_levels), std::move(forced_states), {});

    SECTION("Safi Shattersplitter + 1 Armour") {

        WeaponInstance weapon(db.weapons.at("SAFI_SHATTERSPLITTER"));
        ArmourEquips armour = get_armour("Teostra", "MB",
                                         "", "",
                                         "", "",
                                         "", "",
                                         "", "",
                                         "",
                                         db);
        DecoEquips decos;

        const EffectiveDamageValues edv = calculate_edv_from_gear_lookup(weapon, armour, decos, misc_buffs, skill_spec);
        const double efr = edv.efr;
        REQUIRE(Utils::round_2decpl(efr) == Approx(390.31));
    }

    SECTION("Safi Shattersplitter + 3 Armour") {

        WeaponInstance weapon(db.weapons.at("SAFI_SHATTERSPLITTER"));
        ArmourEquips armour = get_armour("Teostra",       "MB",
                                         "Raging Brachy", "MB",
                                         "Teostra",       "MB",
                                         "", "",
                                         "", "",
                                         "",
                                         db);
        DecoEquips decos;

        const EffectiveDamageValues edv = calculate_edv_from_gear_lookup(weapon, armour, decos, misc_buffs, skill_spec);
        const double efr = edv.efr;
        REQUIRE(Utils::round_2decpl(efr) == Approx(415.77));
    }

    SECTION("Safi Shattersplitter + 5 Armour") {
        WeaponInstance weapon(db.weapons.at("SAFI_SHATTERSPLITTER"));
        ArmourEquips armour = get_armour("Teostra",       "MB",
                                         "Raging Brachy", "MB",
                                         "Teostra",       "MB",
                                         "Teostra",       "MB",
                                         "Raging Brachy", "MB",
                                         "",
                                         db);
        DecoEquips decos;

        const EffectiveDamageValues edv = calculate_edv_from_gear_lookup(weapon, armour, decos, misc_buffs, skill_spec);
        const double efr = edv.efr;
        REQUIRE(Utils::round_2decpl(efr) == Approx(435.11));
    }

    SECTION("Safi Shattersplitter + 5 Armour + Charm") {
        WeaponInstance weapon(db.weapons.at("SAFI_SHATTERSPLITTER"));
        ArmourEquips armour = get_armour("Teostra",       "MB",
                                         "Raging Brachy", "MB",
                                         "Teostra",       "MB",
                                         "Teostra",       "MB",
                                         "Raging Brachy", "MB",
                                         "CHALLENGER_CHARM",
                                         db);
        DecoEquips decos;

        const EffectiveDamageValues edv = calculate_edv_from_gear_lookup(weapon, armour, decos, misc_buffs, skill_spec);
        const double efr = edv.efr;
        REQUIRE(Utils::round_2decpl(efr) == Approx(480.30));
    }

    SECTION("Safi Shattersplitter + Augments + 5 Armour + Charm") {
        WeaponInstance weapon(db.weapons.at("SAFI_SHATTERSPLITTER"));
        weapon.augments->set_augment(WeaponAugment::augment_lvl, 3);
        weapon.augments->set_augment(WeaponAugment::affinity_increase, 1);
        weapon.augments->set_augment(WeaponAugment::health_regen, 1);
        ArmourEquips armour = get_armour("Teostra",       "MB",
                                         "Raging Brachy", "MB",
                                         "Teostra",       "MB",
                                         "Teostra",       "MB",
                                         "Raging Brachy", "MB",
                                         "CHALLENGER_CHARM",
                                         db);
        DecoEquips decos;

        const EffectiveDamageValues edv = calculate_edv_from_gear_lookup(weapon, armour, decos, misc_buffs, skill_spec);
        const double efr = edv.efr;
        REQUIRE(Utils::round_2decpl(efr) == Approx(490.63));
    }

    SECTION("Safi Shattersplitter + Augments + Upgrades + 5 Armour + Charm") {
        WeaponInstance weapon(db.weapons.at("SAFI_SHATTERSPLITTER"));
        weapon.augments->set_augment(WeaponAugment::augment_lvl, 3);
        weapon.augments->set_augment(WeaponAugment::affinity_increase, 1);
        weapon.augments->set_augment(WeaponAugment::health_regen, 1);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_attack_6);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_attack_5);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_attack_5);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_attack_5);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_sharpness_5);
        ArmourEquips armour = get_armour("Teostra",       "MB",
                                         "Raging Brachy", "MB",
                                         "Teostra",       "MB",
                                         "Teostra",       "MB",
                                         "Raging Brachy", "MB",
                                         "CHALLENGER_CHARM",
                                         db);
        DecoEquips decos;

        const EffectiveDamageValues edv = calculate_edv_from_gear_lookup(weapon, armour, decos, misc_buffs, skill_spec);
        const double efr = edv.efr;
        REQUIRE(Utils::round_2decpl(efr) == Approx(554.90));
    }

    SECTION("Safi Shattersplitter + Augments + Upgrades + 5 Armour + Charm + Decorations") {
        WeaponInstance weapon(db.weapons.at("SAFI_SHATTERSPLITTER"));
        weapon.augments->set_augment(WeaponAugment::augment_lvl, 3);
        weapon.augments->set_augment(WeaponAugment::affinity_increase, 1);
        weapon.augments->set_augment(WeaponAugment::health_regen, 1);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_attack_6);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_attack_5);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_attack_5);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_attack_5);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_sharpness_5);
        ArmourEquips armour = get_armour("Teostra",       "MB",
                                         "Raging Brachy", "MB",
                                         "Teostra",       "MB",
                                         "Teostra",       "MB",
                                         "Raging Brachy", "MB",
                                         "CHALLENGER_CHARM",
                                         db);
        DecoEquips decos;
        decos.add(DecorationsDatabase::get_decoration("CHARGER_VITALITY_COMPOUND"));
        decos.add(DecorationsDatabase::get_decoration("CHARGER_VITALITY_COMPOUND"));
        decos.add(DecorationsDatabase::get_decoration("CHARGER_VITALITY_COMPOUND"));
        decos.add(DecorationsDatabase::get_decoration("CRITICAL"));
        decos.add(DecorationsDatabase::get_decoration("ATTACK"));
        decos.add(DecorationsDatabase::get_decoration("ATTACK_2X"));
        decos.add(DecorationsDatabase::get_decoration("CRITICAL"));
        decos.add(DecorationsDatabase::get_decoration("HANDICRAFT_2X"));
        decos.add(DecorationsDatabase::get_decoration("CRITICAL"));
        decos.add(DecorationsDatabase::get_decoration("ATTACK"));
        decos.add(DecorationsDatabase::get_decoration("HANDICRAFT_2X"));
        decos.add(DecorationsDatabase::get_decoration("ATTACK"));
        decos.add(DecorationsDatabase::get_decoration("ATTACK"));

        const EffectiveDamageValues edv = calculate_edv_from_gear_lookup(weapon, armour, decos, misc_buffs, skill_spec);
        const double efr = edv.efr;
        // Interestingly, this assertion fails when we use the -Ofast compiler flag.
        REQUIRE(Utils::round_2decpl(efr) == Approx(682.55));
    }

}


TEST_CASE("Incrementally building up a greatsword Acid Shredder II build.") {

    std::unordered_map<const Skill*, unsigned int> min_levels = {
        {&SkillsDatabase::g_skill_agitator, 0},
        {&SkillsDatabase::g_skill_critical_eye, 0},
        {&SkillsDatabase::g_skill_weakness_exploit, 0},
    };
    std::unordered_map<const Skill*, unsigned int> forced_states = {
        {&SkillsDatabase::g_skill_weakness_exploit, 1},
    };
    MiscBuffsEquips misc_buffs ({
        &MiscBuffsDatabase::get_miscbuff("POWERCHARM"),
        &MiscBuffsDatabase::get_miscbuff("POWERTALON"),
    });
    SkillSpec skill_spec(std::move(min_levels), std::move(forced_states), {});

    DecoEquips decos;

    SECTION("Acid Shredder II + 1 Armour + Charm") {

        WeaponInstance weapon(db.weapons.at("ACID_SHREDDER_II"));
        ArmourEquips armour = get_armour("Teostra", "MB",
                                         "", "",
                                         "", "",
                                         "", "",
                                         "", "",
                                         "ADAMANTINE_CHARM",
                                         db);
        DecoEquips decos;

        const EffectiveDamageValues edv = calculate_edv_from_gear_lookup(weapon, armour, decos, misc_buffs, skill_spec);
        const double efr = edv.efr;
        REQUIRE(Utils::round_2decpl(efr) == Approx(393.60));
    }

    SECTION("Acid Shredder II + Upgrades + 5 Armour + Charm") {

        WeaponInstance weapon(db.weapons.at("ACID_SHREDDER_II"));
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_affinity);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_attack);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_affinity);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_attack);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_affinity);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_attack);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_affinity);
        ArmourEquips armour = get_armour("Raging Brachy", "MB",
                                         "Raging Brachy", "MB",
                                         "Raging Brachy", "MB",
                                         "Raging Brachy", "MB",
                                         "Yian Garuga",   "MB",
                                         "ADAMANTINE_CHARM",
                                         db);
        DecoEquips decos;

        const EffectiveDamageValues edv = calculate_edv_from_gear_lookup(weapon, armour, decos, misc_buffs, skill_spec);
        const double efr = edv.efr;
        REQUIRE(Utils::round_2decpl(efr) == Approx(469.64));
    }

    SECTION("Acid Shredder II + Augments + Upgrades5 Armour + Charm") {

        WeaponInstance weapon(db.weapons.at("ACID_SHREDDER_II"));
        weapon.augments->set_augment(WeaponAugment::augment_lvl, 3);
        weapon.augments->set_augment(WeaponAugment::affinity_increase, 2);
        weapon.augments->set_augment(WeaponAugment::attack_increase, 1);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_affinity);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_attack);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_affinity);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_attack);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_affinity);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_attack);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_cust_affinity);
        ArmourEquips armour = get_armour("Raging Brachy", "MB",
                                         "Raging Brachy", "MB",
                                         "Raging Brachy", "MB",
                                         "Raging Brachy", "MB",
                                         "Yian Garuga",   "MB",
                                         "ADAMANTINE_CHARM",
                                         db);
        DecoEquips decos;

        const EffectiveDamageValues edv = calculate_edv_from_gear_lookup(weapon, armour, decos, misc_buffs, skill_spec);
        const double efr = edv.efr;
        REQUIRE(Utils::round_2decpl(efr) == Approx(492.35));
    }

}


TEST_CASE("Testing unusual skill combinations.") {

    std::unordered_map<const Skill*, unsigned int> min_levels = {
        {&SkillsDatabase::g_skill_weakness_exploit, 0},
        {&SkillsDatabase::g_skill_agitator, 0},
        {&SkillsDatabase::g_skill_fortify, 0},
        {&SkillsDatabase::g_skill_frostcraft, 0},
        {&SkillsDatabase::g_skill_bludgeoner, 0},
        {&SkillsDatabase::g_skill_heroics, 0},
        {&SkillsDatabase::g_skill_airborne, 0},
    };
    std::unordered_map<const Skill*, unsigned int> forced_states = {
        {&SkillsDatabase::g_skill_fortify, 1},
    };
    MiscBuffsEquips misc_buffs ({
        &MiscBuffsDatabase::get_miscbuff("POWERCHARM"),
        &MiscBuffsDatabase::get_miscbuff("POWERTALON"),
    });
    SkillSpec skill_spec(std::move(min_levels), std::move(forced_states), {});

    SECTION("Bludgeoner + Non-elemental Boost + Fortify + Frostcraft + Heroics") {

        WeaponInstance weapon(db.weapons.at("BUSTER_SWORD_I"));

        SkillMap skills;
        skills.set(&SkillsDatabase::g_skill_agitator, 1);
        skills.set(&SkillsDatabase::g_skill_critical_boost, 3);
        skills.set(&SkillsDatabase::g_skill_non_elemental_boost, 1);
        skills.set(&SkillsDatabase::g_skill_fortify, 1);
        skills.set(&SkillsDatabase::g_skill_frostcraft, 1);
        skills.set(&SkillsDatabase::g_skill_bludgeoner, 1);
        skills.set(&SkillsDatabase::g_skill_heroics, 5);

        WeaponContribution wc = weapon.calculate_contribution();

        const EffectiveDamageValues edv = calculate_edv_from_skills_lookup(weapon.weapon->weapon_class,
                                                                           wc,
                                                                           skills,
                                                                           misc_buffs,
                                                                           skill_spec);
        const double efr = edv.efr;
        REQUIRE(Utils::round_2decpl(efr) == Approx(209.51));
    }

    SECTION("Bludgeoner + Non-elemental Boost + Fortify + Frostcraft + Heroics + Airborne (Expecting Raw Overcap)") {

        WeaponInstance weapon(db.weapons.at("BUSTER_SWORD_I"));

        SkillMap skills;
        skills.set(&SkillsDatabase::g_skill_agitator, 1);
        skills.set(&SkillsDatabase::g_skill_critical_boost, 3);
        skills.set(&SkillsDatabase::g_skill_non_elemental_boost, 1);
        skills.set(&SkillsDatabase::g_skill_fortify, 1);
        skills.set(&SkillsDatabase::g_skill_frostcraft, 1);
        skills.set(&SkillsDatabase::g_skill_bludgeoner, 1);
        skills.set(&SkillsDatabase::g_skill_heroics, 5);
        skills.set(&SkillsDatabase::g_skill_airborne, 1);

        WeaponContribution wc = weapon.calculate_contribution();

        const EffectiveDamageValues edv = calculate_edv_from_skills_lookup(weapon.weapon->weapon_class,
                                                                           wc,
                                                                           skills,
                                                                           misc_buffs,
                                                                           skill_spec);
        const double efr = edv.efr;
        REQUIRE(Utils::round_2decpl(efr) == Approx(212.16));
    }
}


TEST_CASE("DecoEquips::fits()") {

    std::unordered_map<const Skill*, unsigned int> min_levels = {
        {&SkillsDatabase::g_skill_agitator, 0},
        {&SkillsDatabase::g_skill_coalescence, 0},
        {&SkillsDatabase::g_skill_peak_performance, 0},
        {&SkillsDatabase::g_skill_weakness_exploit, 0},
    };
    std::unordered_map<const Skill*, unsigned int> forced_states;
    MiscBuffsEquips misc_buffs ({
        &MiscBuffsDatabase::get_miscbuff("POWERCHARM"),
        &MiscBuffsDatabase::get_miscbuff("POWERTALON"),
    });
    SkillSpec skill_spec(std::move(min_levels), std::move(forced_states), {});

    SECTION("Mixed Test, Exact Fit") {

        WeaponInstance weapon(db.weapons.at("SAFI_SHATTERSPLITTER"));
        weapon.augments->set_augment(WeaponAugment::augment_lvl, 3);
        weapon.augments->set_augment(WeaponAugment::attack_increase, 1);
        weapon.augments->set_augment(WeaponAugment::slot_upgrade, 1);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_attack_6);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_deco_slot_3);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_affinity_5);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_sharpness_5);
        weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_sharpness_4);
        ArmourEquips armour = get_armour("", "", // EMPTY HEAD
                                         "Teostra",     "MB",
                                         "Teostra",     "MB",
                                         "Teostra",     "MA",
                                         "Yian Garuga", "MB",
                                         "UNSCATHED_CHARM",
                                         db);
        DecoEquips decos;

        const EffectiveDamageValues edv = calculate_edv_from_gear_lookup(weapon, armour, decos, misc_buffs, skill_spec);
        const double efr = edv.efr;
        REQUIRE(decos.fits_in(armour, weapon.calculate_contribution()));
        REQUIRE(Utils::round_2decpl(efr) == Approx(512.28));

        decos.add(DecorationsDatabase::get_decoration("CHALLENGER_2X"));
        decos.add(DecorationsDatabase::get_decoration("ATTACK"));
        decos.add(DecorationsDatabase::get_decoration("PHOENIX"));
        decos.add(DecorationsDatabase::get_decoration("ATTACK_2X"));
        decos.add(DecorationsDatabase::get_decoration("ATTACK"));
        decos.add(DecorationsDatabase::get_decoration("EXPERT"));
        decos.add(DecorationsDatabase::get_decoration("EXPERT_2X"));
        decos.add(DecorationsDatabase::get_decoration("EARPLUG"));
        decos.add(DecorationsDatabase::get_decoration("PHOENIX"));
        decos.add(DecorationsDatabase::get_decoration("CHALLENGER_VITALITY_COMPOUND"));
        decos.add(DecorationsDatabase::get_decoration("CRITICAL"));
        decos.add(DecorationsDatabase::get_decoration("CRITICAL"));

        const EffectiveDamageValues edv2 = calculate_edv_from_gear_lookup(weapon, armour, decos, misc_buffs, skill_spec);
        const double efr2 = edv2.efr;
        REQUIRE(decos.fits_in(armour, weapon.calculate_contribution()));
        REQUIRE(Utils::round_2decpl(efr2) == Approx(649.38));

        // Now, we attempt to overflow the deco capacity.

        DecoEquips new_decos = decos;
        new_decos.add(DecorationsDatabase::get_decoration("ATTACK"));
        REQUIRE(!new_decos.fits_in(armour, weapon.calculate_contribution()));

        new_decos = decos;
        new_decos.add(DecorationsDatabase::get_decoration("ELEMENTLESS"));
        REQUIRE(!new_decos.fits_in(armour, weapon.calculate_contribution()));

        new_decos = decos;
        new_decos.add(DecorationsDatabase::get_decoration("HANDICRAFT"));
        REQUIRE(!new_decos.fits_in(armour, weapon.calculate_contribution()));

        new_decos = decos;
        new_decos.add(DecorationsDatabase::get_decoration("TENDERIZER_VITALITY_COMPOUND"));
        REQUIRE(!new_decos.fits_in(armour, weapon.calculate_contribution()));

    }

}


} // namespace

