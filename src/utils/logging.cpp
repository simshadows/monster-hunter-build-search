/*
 * File: logging.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
//#include <fstream>
#include <iostream>

#include "utils.h"



namespace Utils
{


static std::vector<std::string> stats_to_reprint;


void log_stat(const std::string& s) {
    std::clog << s << std::endl;
    stats_to_reprint.emplace_back(s);
}

void log_stat(const std::string& s, const int v) {
    std::string msg = s + std::to_string(v);
    std::clog << msg << std::endl;
    stats_to_reprint.emplace_back(std::move(msg));
}


} // namespace

