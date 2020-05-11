/*
 * File: database.cpp
 * Author: <contact@simshadows.com>
 */

#include "database.h"

namespace MHWIBuildSearch {


// Constructor
const Database Database::get_db() {
    return Database();
}


Database::Database()
    : skills  (SkillsDatabase     ::read_db_file("data/database_skills.json"             ))
    , decos   (DecorationsDatabase::read_db_file("data/database_decorations.json", skills))
    , weapons (WeaponsDatabase    ::read_db_file("data/database_weapons.json"    , skills))
    , armour  (ArmourDatabase     ::read_db_file("data/database_armour.json"     , skills))
    , charms  (CharmsDatabase     ::read_db_file("data/database_charms.json"     , skills))

    , affinity_sliding_ptr          (skills.skill_at("AFFINITY_SLIDING"         ))
    , agitator_ptr                  (skills.skill_at("AGITATOR"                 ))
    , agitator_secret_ptr           (skills.skill_at("AGITATOR_SECRET"          ))
    , attack_boost_ptr              (skills.skill_at("ATTACK_BOOST"             ))
    , coalescence_ptr               (skills.skill_at("COALESCENCE"              ))
    , critical_boost_ptr            (skills.skill_at("CRITICAL_BOOST"           ))
    , critical_draw_ptr             (skills.skill_at("CRITICAL_DRAW"            ))
    , critical_eye_ptr              (skills.skill_at("CRITICAL_EYE"             ))
    , dragonvein_awakening_ptr      (skills.skill_at("DRAGONVEIN_AWAKENING"     ))
    , element_acceleration_ptr      (skills.skill_at("ELEMENT_ACCELERATION"     ))
    , free_elem_ammo_up_ptr         (skills.skill_at("FREE_ELEM_AMMO_UP"        ))
    , handicraft_ptr                (skills.skill_at("HANDICRAFT"               ))
    , latent_power_ptr              (skills.skill_at("LATENT_POWER"             ))
    , latent_power_secret_ptr       (skills.skill_at("LATENT_POWER_SECRET"      ))
    , maximum_might_ptr             (skills.skill_at("MAXIMUM_MIGHT"            ))
    , maximum_might_secret_ptr      (skills.skill_at("MAXIMUM_MIGHT_SECRET"     ))
    , non_elemental_boost_ptr       (skills.skill_at("NON_ELEMENTAL_BOOST"      ))
    , peak_performance_ptr          (skills.skill_at("PEAK_PERFORMANCE"         ))
    , resentment_ptr                (skills.skill_at("RESENTMENT"               ))
    , true_dragonvein_awakening_ptr (skills.skill_at("TRUE_DRAGONVEIN_AWAKENING"))
    , true_element_acceleration_ptr (skills.skill_at("TRUE_ELEMENT_ACCELERATION"))
    , weakness_exploit_ptr          (skills.skill_at("WEAKNESS_EXPLOIT"         ))
{
}


} // namespace

