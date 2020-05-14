/*
 * File: utils.h
 * Author: <contact@simshadows.com>
 */

#ifndef MHWIBS_UTILS_H
#define MHWIBS_UTILS_H

#include <iostream>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace Utils {


/*
 * Logging
 */


void log_stat(const std::string& s="");
void log_stat(const std::string& s, int v);
void log_stat_reduction(const std::string& s, const int v_before, const int v_after);


/*
 * Inlines
 */


inline double round_2decpl(const double v) {
    return std::round(v * 100) / 100;
}

//inline bool equal_within_2decpl(const double a, const double b) {
//    return std::round(a * 100) == std::round(b * 100);
//}

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

// TODO: Implement a general solution if I happen to need more numerals.
inline std::string to_capital_roman_numerals(unsigned int v) {
    switch (v) {
        case 0:
            throw std::logic_error("Zero cannot be represented as a roman numeral.");
        case 1: return "I";
        case 2: return "II";
        case 3: return "III";
        case 4: return "IV";
        case 5: return "V";
        case 6: return "VI";
        case 7: return "VII";
        default:
            throw std::logic_error("Roman numerals above VII (7) have not been implemented yet.");
    }
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


// TODO: Figure out how the actual set diff function works.
template<class S = std::unordered_set<class K>>
inline S set_diff(const S& s1, const S& s2) {
    S ret;
    for (const auto& e : s1) {
        if (!set_has_key(s2, e)) ret.insert(e);
    }
    return ret;
}


} // namespace

#endif // MHWIBS_UTILS_H

