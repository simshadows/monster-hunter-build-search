/*
 * File: database.cpp
 * Author: <contact@simshadows.com>
 */

#include "database.h"

namespace Database {


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

    , agitator_ptr            (skills.skill_at("AGITATOR"           ))
    , agitator_secret_ptr     (skills.skill_at("AGITATOR_SECRET"    ))
    , attack_boost_ptr        (skills.skill_at("ATTACK_BOOST"       ))
    , critical_boost_ptr      (skills.skill_at("CRITICAL_BOOST"     ))
    , critical_eye_ptr        (skills.skill_at("CRITICAL_EYE"       ))
    , handicraft_ptr          (skills.skill_at("HANDICRAFT"         ))
    , non_elemental_boost_ptr (skills.skill_at("NON_ELEMENTAL_BOOST"))
{
}


} // namespace

