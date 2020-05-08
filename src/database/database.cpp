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
    : skills  (SkillsDatabase ::read_db_file("data/database_skills.json"         ))
    , weapons (WeaponsDatabase::read_db_file("data/database_weapons.json", skills))
    , armour  (ArmourDatabase ::read_db_file("data/database_armour.json" , skills))
    , charms  (CharmsDatabase ::read_db_file("data/database_charms.json" , skills))

    , critical_boost_ptr      (skills.skill_at("CRITICAL_BOOST"     ))
    , handicraft_ptr          (skills.skill_at("HANDICRAFT"         ))
    , non_elemental_boost_ptr (skills.skill_at("NON_ELEMENTAL_BOOST"))
{
}


} // namespace

