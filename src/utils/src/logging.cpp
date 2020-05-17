/*
 * File: logging.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
//#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>

#include "../logging.h"


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


void log_stat_duration(const std::string& s, const std::chrono::steady_clock::time_point& time_start) {
    const std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();

    const double total_sec = std::chrono::duration<double>(time_end - time_start).count();
    assert(total_sec > 0);

    const unsigned int min = total_sec / 60;
    const double sec = std::fmod(total_sec, 60);

    std::string sec_str = std::to_string(sec);
    if (sec < 10) sec_str = "0" + sec_str; // ugh, too hackish.

    std::string msg = s + std::to_string(min) + "m" + sec_str + "s";
    std::clog << msg << std::endl;
    stats_to_reprint.emplace_back(std::move(msg));
}


} // namespace

