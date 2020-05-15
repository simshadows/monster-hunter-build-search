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

void log_stat_reduction(const std::string& s, const int v_before, const int v_after) {
    assert(v_before >= v_after);
    const double kept = (((double)v_after) / v_before) * 100;
    std::string msg = s
                      + std::to_string(v_before)
                      + " --> "
                      + std::to_string(v_after)
                      + " ("
                      + std::to_string(kept)
                      + "% kept)";
    std::clog << msg << std::endl;
    stats_to_reprint.emplace_back(std::move(msg));
}


} // namespace

