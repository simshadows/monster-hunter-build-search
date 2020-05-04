/*
 * File: utils.h
 * Author: <contact@simshadows.com>
 */

#ifndef MHWIBS_UTILS_H
#define MHWIBS_UTILS_H

#include <iostream>
#include <cmath>

namespace Utils {


inline double round_2decpl(const double v) {
    return std::round(v * 100) / 100;
}

inline bool is_upper_snake_case(const std::string& s) noexcept {
    for (const char e : s) {
        if ((e >= 'A' && e <= 'Z') or (e == '_')) continue;
        return false;
    }
    return true;
}


} // namespace

#endif // MHWIBS_UTILS_H

