/*
 * File: database.cpp
 * Author: <contact@simshadows.com>
 */

#include "database.h"

namespace Database {


static constexpr char k_WEAPONS_DB_FILEPATH[] = "data/database_weapons.json";
static constexpr char k_SKILLS_DB_FILEPATH[]  = "data/database_skills.json";


// Constructor
const Database Database::get_db() {
    return Database();
}


Database::Database()
    : skills (SkillsDatabase::read_db_file(k_SKILLS_DB_FILEPATH))
    , weapons (WeaponsDatabase::read_db_file(k_WEAPONS_DB_FILEPATH, skills))

    , critical_boost_ptr      (skills.skill_at("CRITICAL_BOOST"     ))
    , handicraft_ptr          (skills.skill_at("HANDICRAFT"         ))
    , non_elemental_boost_ptr (skills.skill_at("NON_ELEMENTAL_BOOST"))
{
}


} // namespace

