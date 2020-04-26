/*
 * File: utils.cpp
 * Author: <contact@simshadows.com>
 */

#include "utils.h"

#include <cmath>

namespace Utils {


double round_2decpl(double v) {
    return std::round(v * 100) / 100;
}


} // end of namespace

