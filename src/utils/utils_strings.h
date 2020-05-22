/*
 * File: utils_strings.h
 * Author: <contact@simshadows.com>
 */

#ifndef UTILS_STRING_H
#define UTILS_STRING_H

#include <assert.h>
#include <string>
#include <regex>

namespace Utils {


inline std::string indent(const std::string& s, const std::size_t num_spaces) noexcept {
    const std::string spaces (num_spaces, ' ');
    return std::regex_replace(spaces + s, std::regex("\n"), "\n" + spaces);
}


// TODO: Should I be passing in by copy, or passing in by const reference?
template<class ForwardIt>
inline std::string str_join(const ForwardIt& first, const ForwardIt& last, const std::string& sep) noexcept {
    std::string ret;
    bool is_first = true;
    for (ForwardIt p = first; p < last; ++p) {
        if (!is_first) ret += sep;
        ret += *p;
        is_first = false;
    }
    return ret;
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


} // namespace

#endif // UTILS_STRING_H

