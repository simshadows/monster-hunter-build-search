/*
 * File: database.cpp
 * Author: <contact@simshadows.com>
 */

#include "../database.h"

namespace MHWIBuildSearch {


// Constructor
const Database Database::get_db() {
    return Database();
}


Database::Database()
    : weapons (WeaponsDatabase    ::read_db_file("data/database_weapons.json"    ))
    , armour  (ArmourDatabase     ::read_db_file("data/database_armour.json"     ))
    , charms  (CharmsDatabase     ::read_db_file("data/database_charms.json"     ))
{
}


} // namespace

