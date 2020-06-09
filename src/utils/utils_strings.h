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


// I'll be using this random thing I found on stackoverflow for now until I find a better
// implementation, ideally from the standard library.
inline std::size_t utf8_strlen(const std::string& s) noexcept {
    int len = 0;
    for (const char e : s) {
        len += (e & 0xc0) != 0x80;
    }
    return len;
}


inline std::string indent(const std::string& s, const std::size_t num_spaces) noexcept {
    const std::string spaces (num_spaces, ' ');
    return std::regex_replace(spaces + s, std::regex("\n"), "\n" + spaces);
}


// Splits at '\n'. Newline characters are NOT retained in the returned vector.
inline std::vector<std::string> split_and_remove_newline(const std::string& s) noexcept {
    std::vector<std::string> ret;
    auto start_of_line = s.begin();
    for (auto p = s.begin(); p < s.end(); ++p) {
        if (*p == '\n') {
            ret.emplace_back(start_of_line, p);
            start_of_line = p + 1;
        }
    }
    // Even if the final character is a newline, we still split it off into a new empty line.
    ret.emplace_back(start_of_line, s.end());
    return ret;
}


// Naive implementation. We can most likely make this more efficient.
inline std::string two_column_text(const std::string& left_column,
                                   const std::string& right_column,
                                   const std::string& middle_separator) noexcept {

    std::vector<std::string> left_lines = split_and_remove_newline(left_column);
    std::vector<std::string> right_lines = split_and_remove_newline(right_column);

    const std::size_t left_width = [&](){
        std::size_t longest_line_length = 0;
        for (const std::string& e : left_lines) {
            const std::size_t curr_line_length = utf8_strlen(e);
            if (curr_line_length > longest_line_length) longest_line_length = curr_line_length;
        }
        return longest_line_length;
    }();

    // We pad the shortest vector.
    // (I don't like this, but it's easy, so we'll keep it for now.)
    if (left_lines.size() > right_lines.size()) {
        const std::size_t diff = left_lines.size() - right_lines.size();
        right_lines.insert(right_lines.end(), diff, "");
    } else {
        const std::size_t diff = right_lines.size() - left_lines.size();
        left_lines.insert(left_lines.end(), diff, "");
    }
    assert(left_lines.size() == right_lines.size());

    std::string ret;

    auto lp = left_lines.begin();
    auto rp = right_lines.begin();
    bool is_first = true;
    for (; lp < left_lines.end();) {
        if (!is_first) ret += "\n";
        {
            assert(left_width >= utf8_strlen(*lp));
            const std::size_t padding_length = left_width - utf8_strlen(*lp);
            ret += *lp;
            ret.append(padding_length, ' ');
            ret += middle_separator + *rp;
        }
        ++lp;
        ++rp;
        is_first = false;
    }
    assert(lp == left_lines.end());
    assert(rp == right_lines.end());

    return ret;
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

