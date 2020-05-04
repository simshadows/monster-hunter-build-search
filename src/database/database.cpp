/*
 * File: database.cpp
 * Author: <contact@simshadows.com>
 */

#include "database.h"

namespace Database {


static constexpr char k_WEAPONS_DB_FILEPATH[] = "data/database_weapons.json";



// Constructor
const Database Database::get_db() {
    return Database();
}


Database::Database() noexcept
    : weapons (WeaponsDatabase::read_weapon_db_file(k_WEAPONS_DB_FILEPATH))
{
}


} // namespace

