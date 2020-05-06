/*
 * File: utils.h
 * Author: <contact@simshadows.com>
 */

#ifndef MHWIBS_UTILS_H
#define MHWIBS_UTILS_H

#include <iostream>
#include <cmath>
#include <unordered_map>
#include <unordered_set>

namespace Utils {


inline double round_2decpl(const double v) {
    return std::round(v * 100) / 100;
}

inline bool is_upper_snake_case(const std::string& s) noexcept {
    for (const char e : s) {
        if ((e >= 'A' && e <= 'Z') || (e >= '0' && e <= '9') || (e == '_')) continue;
        return false;
    }
    return true;
}

inline bool has_ascii_letters(const std::string& s) noexcept {
    for (const char e : s) {
        if ((e >= 'A' && e <= 'Z') || (e >= 'a' && e <= 'z')) {
            return true;
        }
    }
    return false; // Reached the end of the string without seeing an ASCII letter.
}

// TODO: Eventually replace this with the C++20 std::unordered_map::contains method.
template<class K,
         class M = std::unordered_map<K, class V>>
inline bool map_has_key(const M& m, const K& k) {
    auto search = m.find(k);
    return search != m.end();
}

// TODO: Eventually replace this with the C++20 std::unordered_set::contains method.
template<class K,
         class S = std::unordered_set<K>>
inline bool set_has_key(const S& s, const K& k) {
    auto search = s.find(k);
    return search != s.end();
}


} // namespace

#endif // MHWIBS_UTILS_H

