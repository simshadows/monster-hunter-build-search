/*
 * File: run_tests.cpp
 * Author: <contact@simshadows.com>
 *
 * Testing for MHWI Build Search.
 */

#define CATCH_CONFIG_MAIN
#include "../dependencies/catch-2-12-1/catch.hpp"

#include "../src/core/core.h"
#include "../src/database/database.h"
#include "../src/support/support.h"
#include "../src/utils.h"

namespace TestMHWIBuildSearch
{
using namespace MHWIBuildSearch;


TEST_CASE("Computing the EFR of a greatsword build.") {

    const Database db = Database::get_db();

    std::unordered_map<const Skill*, unsigned int> min_levels = {
        {db.weakness_exploit_ptr, 0},
        {db.agitator_ptr, 0},
    };
    std::unordered_map<const Skill*, unsigned int> forced_states;
    SkillSpec skill_spec(std::move(min_levels), std::move(forced_states));

    WeaponInstance weapon(db.weapons.at("SAFI_SHATTERSPLITTER"));
    weapon.augments->set_augment(WeaponAugment::augment_lvl, 3);
    weapon.augments->set_augment(WeaponAugment::attack_increase, 1);
    weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_sharpness_6);
    weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_sharpness_5);
    weapon.upgrades->add_upgrade(WeaponUpgrade::ib_safi_sharpness_5);

    ArmourEquips armour;
    armour.add(db.armour.at("Raging Brachy",
                            Tier::master_rank,
                            ArmourVariant::master_rank_beta_plus,
                            ArmourSlot::head));
    armour.add(db.armour.at("Teostra",
                            Tier::master_rank,
                            ArmourVariant::master_rank_beta_plus,
                            ArmourSlot::arms));
    armour.add(db.charms.at("CHALLENGER_CHARM"));

    double efr = calculate_efr_from_gear_lookup(db, weapon, armour, skill_spec);
    REQUIRE(Utils::round_2decpl(efr) == 479.38);

}


} // namespace

