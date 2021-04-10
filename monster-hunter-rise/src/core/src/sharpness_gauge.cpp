/*
 * File: sharpness_gauge.cpp
 * Author: <contact@simshadows.com>
 */

#include <stdexcept>
#include <assert.h>

#include "../core.h"

namespace MHRBuildSearch
{


static constexpr unsigned int k_HANDICRAFT_MAX = 5;

static constexpr double k_RAW_SHARPNESS_MODIFIER_RED    = 0.50;
static constexpr double k_RAW_SHARPNESS_MODIFIER_ORANGE = 0.75;
static constexpr double k_RAW_SHARPNESS_MODIFIER_YELLOW = 1.00;
static constexpr double k_RAW_SHARPNESS_MODIFIER_GREEN  = 1.05;
static constexpr double k_RAW_SHARPNESS_MODIFIER_BLUE   = 1.20;
static constexpr double k_RAW_SHARPNESS_MODIFIER_WHITE  = 1.32;
static constexpr double k_RAW_SHARPNESS_MODIFIER_PURPLE = 1.39;

static constexpr double k_ELEMENTAL_SHARPNESS_MODIFIER_RED    = 0.25;
static constexpr double k_ELEMENTAL_SHARPNESS_MODIFIER_ORANGE = 0.50;
static constexpr double k_ELEMENTAL_SHARPNESS_MODIFIER_YELLOW = 0.75;
static constexpr double k_ELEMENTAL_SHARPNESS_MODIFIER_GREEN  = 1.00;
static constexpr double k_ELEMENTAL_SHARPNESS_MODIFIER_BLUE   = 1.0625;
static constexpr double k_ELEMENTAL_SHARPNESS_MODIFIER_WHITE  = 1.15;
static constexpr double k_ELEMENTAL_SHARPNESS_MODIFIER_PURPLE = 1.25;

static std::string sharpness_level_to_humanreadable(const SharpnessLevel lvl) {
    switch (lvl) {
        case SharpnessLevel::red:    return "Red";
        case SharpnessLevel::orange: return "Orange";
        case SharpnessLevel::yellow: return "Yellow";
        case SharpnessLevel::green:  return "Green";
        case SharpnessLevel::blue:   return "Blue";
        case SharpnessLevel::white:  return "White";
        case SharpnessLevel::purple: return "Purple";
        default:
            throw std::logic_error("Invalid sharpness level.");
    }
}


SharpnessGauge::SharpnessGauge(unsigned int r,
                               unsigned int o,
                               unsigned int y,
                               unsigned int g,
                               unsigned int b,
                               unsigned int w,
                               unsigned int p) noexcept
    : hits (std::array<unsigned int, k_SHARPNESS_LEVELS> {r, o, y, g, b, w, p})
{
    // Nothing left to do.
}


SharpnessGauge SharpnessGauge::from_vector(const std::vector<unsigned int>& vec) {
    SharpnessGauge ret;
    if (vec.size() != ret.hits.size()) {
        throw std::runtime_error("maximum_sharpness key from input json is an incorrect size.");
    }
    auto hits_end = std::copy(vec.begin(), vec.end(), ret.hits.begin());
    assert(hits_end == ret.hits.end()); // Sanity check
    (void)hits_end;
    return ret;
}


double SharpnessGauge::sharpness_level_to_raw_sharpness_modifier(const SharpnessLevel lvl) {
    switch (lvl) {
        case SharpnessLevel::red:    return k_RAW_SHARPNESS_MODIFIER_RED;
        case SharpnessLevel::orange: return k_RAW_SHARPNESS_MODIFIER_ORANGE;
        case SharpnessLevel::yellow: return k_RAW_SHARPNESS_MODIFIER_YELLOW;
        case SharpnessLevel::green:  return k_RAW_SHARPNESS_MODIFIER_GREEN;
        case SharpnessLevel::blue:   return k_RAW_SHARPNESS_MODIFIER_BLUE;
        case SharpnessLevel::white:  return k_RAW_SHARPNESS_MODIFIER_WHITE;
        case SharpnessLevel::purple: return k_RAW_SHARPNESS_MODIFIER_PURPLE;
        default:
            throw std::logic_error("Invalid sharpness level.");
    }
}


SharpnessGauge SharpnessGauge::apply_handicraft(unsigned int handicraft_lvl) const noexcept {
    assert(handicraft_lvl <= k_HANDICRAFT_MAX);

    SharpnessGauge ret;

    unsigned int hits_to_subtract = (k_HANDICRAFT_MAX - handicraft_lvl) * 10;

    // TODO: Rewrite this unsafe reverse loop.
    assert(ret.hits.size() == this->hits.size());
    for (int i = ret.hits.size() - 1; i >= 0; --i) {
        if (this->hits[i] > hits_to_subtract) {
            ret.hits[i] = this->hits[i] - hits_to_subtract;
            hits_to_subtract = 0;
        } else {
            ret.hits[i] = 0;
            hits_to_subtract -= this->hits[i];
        }
    }

    return ret;
}


double SharpnessGauge::get_raw_sharpness_modifier() const {
    // TODO: Rewrite this unsafe reverse loop.
    for (int i = this->hits.size() - 1; i >= 0; --i) {
        if (this->hits[i] > 0) {
            switch (i) {
                case 0: return k_RAW_SHARPNESS_MODIFIER_RED;
                case 1: return k_RAW_SHARPNESS_MODIFIER_ORANGE;
                case 2: return k_RAW_SHARPNESS_MODIFIER_YELLOW;
                case 3: return k_RAW_SHARPNESS_MODIFIER_GREEN;
                case 4: return k_RAW_SHARPNESS_MODIFIER_BLUE;
                case 5: return k_RAW_SHARPNESS_MODIFIER_WHITE;
                case 6: return k_RAW_SHARPNESS_MODIFIER_PURPLE;
                default:
                    throw std::logic_error("Invalid index.");
            }
        }
    }
    // If the sharpness gauge is ever empty, we will still be in red gauge.
    return k_RAW_SHARPNESS_MODIFIER_RED;
}


double SharpnessGauge::get_elemental_sharpness_modifier() const {
    // TODO: Rewrite this unsafe reverse loop.
    for (int i = this->hits.size() - 1; i >= 0; --i) {
        if (this->hits[i] > 0) {
            switch (i) {
                case 0: return k_ELEMENTAL_SHARPNESS_MODIFIER_RED;
                case 1: return k_ELEMENTAL_SHARPNESS_MODIFIER_ORANGE;
                case 2: return k_ELEMENTAL_SHARPNESS_MODIFIER_YELLOW;
                case 3: return k_ELEMENTAL_SHARPNESS_MODIFIER_GREEN;
                case 4: return k_ELEMENTAL_SHARPNESS_MODIFIER_BLUE;
                case 5: return k_ELEMENTAL_SHARPNESS_MODIFIER_WHITE;
                case 6: return k_ELEMENTAL_SHARPNESS_MODIFIER_PURPLE;
                default:
                    throw std::logic_error("Invalid index.");
            }
        }
    }
    // If the sharpness gauge is ever empty, we will still be in red gauge.
    return k_RAW_SHARPNESS_MODIFIER_RED;
}


SharpnessLevel SharpnessGauge::get_sharpness_level() const {
    // TODO: Rewrite this unsafe reverse loop.
    for (int i = this->hits.size() - 1; i >= 0; --i) {
        if (this->hits[i] > 0) {
            switch (i) {
                case 0: return SharpnessLevel::red;
                case 1: return SharpnessLevel::orange;
                case 2: return SharpnessLevel::yellow;
                case 3: return SharpnessLevel::green;
                case 4: return SharpnessLevel::blue;
                case 5: return SharpnessLevel::white;
                case 6: return SharpnessLevel::purple;
                default:
                    throw std::logic_error("Invalid index.");
            }
        }
    }
    // If the sharpness gauge is ever empty, we will still be in red gauge.
    return SharpnessLevel::red;
}


// TODO: A simple, temporary implementation for now. Reimplement later if performance is required.
double SharpnessGauge::get_raw_sharpness_modifier(unsigned int handicraft_lvl) const {
    const SharpnessGauge modified_gauge = this->apply_handicraft(handicraft_lvl);
    return modified_gauge.get_raw_sharpness_modifier();
}


std::string SharpnessGauge::get_humanreadable() const {
    std::string buf = "";
    bool first = true;
    for (unsigned int v : this->hits) {
        if (!first) buf += " ";
        buf += std::to_string(v);
        first = false;
    }
    buf += " [" + sharpness_level_to_humanreadable(this->get_sharpness_level()) + "]";
    return buf;
}


bool SharpnessGauge::left_has_eq_or_more_hits(const SharpnessGauge& lhs, const SharpnessGauge& rhs) noexcept {
    assert(lhs.hits.size() == rhs.hits.size()); // TODO: Static assert?
    for (std::size_t i = 0; i < lhs.hits.size(); ++i) {
        if (lhs.hits[i] < rhs.hits[i]) return false;
    }
    return true;
}


SharpnessGauge::SharpnessGauge() noexcept
    : hits (std::array<unsigned int, k_SHARPNESS_LEVELS>())
{
}


} // namespace

