/*
 * File: sharpness_gauge.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>

#include "../core.h"

namespace MHWIBuildSearch
{


static constexpr unsigned int k_HANDICRAFT_MAX = 5;

static constexpr double k_RAW_SHARPNESS_MODIFIER_RED    = 0.50;
static constexpr double k_RAW_SHARPNESS_MODIFIER_ORANGE = 0.75;
static constexpr double k_RAW_SHARPNESS_MODIFIER_YELLOW = 1.00;
static constexpr double k_RAW_SHARPNESS_MODIFIER_GREEN  = 1.05;
static constexpr double k_RAW_SHARPNESS_MODIFIER_BLUE   = 1.20;
static constexpr double k_RAW_SHARPNESS_MODIFIER_WHITE  = 1.32;
static constexpr double k_RAW_SHARPNESS_MODIFIER_PURPLE = 1.39;


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


// TODO: Rewrite this function. It's so freaking ugly.
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
    return buf;
}


SharpnessGauge::SharpnessGauge() noexcept
    : hits (std::array<unsigned int, k_SHARPNESS_LEVELS>())
{
}


} // namespace

