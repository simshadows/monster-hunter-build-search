/*
 * File: counter.h
 * Author: <contact@simshadows.com>
 *
 * I'm not actually implementing a "counter" just yet.
 * For now, I'm just supplying helper functions for unordered maps used
 * as counters.
 *
 * If this turns out to be particularly useful, I might make it more
 * general somehow.
 */

#ifndef COUNTER_H
#define COUNTER_H

#include <vector>
#include <unordered_map>

namespace Utils {


template<class T>
std::vector<std::unordered_map<T, unsigned int>> generate_cartesian_product(const std::vector<T>& keys,
                                                                            const unsigned int max_each) noexcept {
    std::vector<std::unordered_map<T, unsigned int>> ret {{}};
    assert(ret.size() == 1); // We start with a seeded collection.

    for (const T& key : keys) {
        auto new_ret = ret;
        for (const auto& old_counter : ret) {
            for (unsigned int v = 1; v <= max_each; ++v) {
                new_ret.emplace_back(old_counter);
                new_ret.back().insert({key, v}); // TODO: Get the new object directly from emplace?
            }
        }
        ret = std::move(new_ret);
    }
    
    return ret;
}


} // namespace

#endif // COUNTER_H

