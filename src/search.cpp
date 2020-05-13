/*
 * File: search.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <iostream>

#include "mhwi_build_search.h"
#include "core/core.h"
#include "database/database.h"
#include "support/support.h"
#include "utils.h"


namespace MHWIBuildSearch
{


static void do_search(const Database& db, const SearchParameters& params) {
    (void)db;
    (void)params;
}


void search_cmd(const std::string& search_parameters_path) {

    const Database db = Database::get_db();
    const SearchParameters params = read_file(db, search_parameters_path);

    do_search(db, params);
}


} // namespace

