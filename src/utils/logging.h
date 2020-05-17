/*
 * File: logging.h
 * Author: <contact@simshadows.com>
 */

#ifndef LOGGING_H
#define LOGGING_H

#include <chrono>

namespace Utils {


void log_stat(const std::string& s="");
void log_stat(const std::string& s, int v);
void log_stat_reduction(const std::string& s, const int v_before, const int v_after);

void log_stat_duration(const std::string& s, const std::chrono::steady_clock::time_point& time_start);


} // namespace

#endif // LOGGING_H

