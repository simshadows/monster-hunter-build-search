/*
 * File: utils.h
 * Author: <contact@simshadows.com>
 */

#include <cmath>

namespace Utils {


double round_2decpl(double v) {
    return std::round(v * 100) / 100;
}


} // namespace

