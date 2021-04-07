/*
 * File: utils.h
 * Author: <contact@simshadows.com>
 */

#ifndef UTILS_H
#define UTILS_H

#include <assert.h>
#include <cmath>
#include <unordered_set>
#include <unordered_map>

namespace Utils {


inline double round_2decpl(const double v) {
    return std::round(v * 100) / 100;
}

inline unsigned int ceil_div(const unsigned int n, const unsigned int d) {
    assert(n <= (std::numeric_limits<unsigned int>::max() - d)); // Addition overflow check
    return (n + d - 1) / d;
}

//inline bool equal_within_2decpl(const double a, const double b) {
//    return std::round(a * 100) == std::round(b * 100);
//}

// TODO: Eventually replace this with the C++20 std::unordered_map::contains method.
template<class K,
         class M = std::unordered_map<K, class V>>
inline bool map_has_key(const M& m, const K& k) {
    const auto result = m.find(k);
    return result != m.end();
}

// TODO: Eventually replace this with the C++20 std::unordered_set::contains method.
template<class K,
         class S = std::unordered_set<K>>
inline bool set_has_key(const S& s, const K& k) {
    const auto result = s.find(k);
    return result != s.end();
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

#endif // UTILS_H

