/*
 * File: database.cpp
 * Author: <contact@simshadows.com>
 */

#include "database.h"

namespace Database {


static constexpr char k_WEAPONS_DB_FILEPATH[] = "data/database_weapons.json";
static constexpr char k_SKILLS_DB_FILEPATH[]  = "data/database_skills.json";

static constexpr char k_HANDICRAFT_ID[] = "HANDICRAFT";


// Constructor
const Database Database::get_db() {
    return Database();
}


Database::Database()
    : skills (SkillsDatabase::read_skills_db_file(k_SKILLS_DB_FILEPATH))
    , weapons (WeaponsDatabase::read_weapon_db_file(k_WEAPONS_DB_FILEPATH, skills))

    , handicraft_ptr (skills.skill_at(k_HANDICRAFT_ID))
{
}


} // namespace

