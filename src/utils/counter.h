/*
 * File: counter.h
 * Author: <contact@simshadows.com>
 *
 * Counter is mostly like the Python collections.Counter class.
 */

#ifndef COUNTER_H
#define COUNTER_H

#include <vector>
#include <unordered_map>

#include "utils.h"

namespace Utils {


template<class T>
struct CounterNoValueClip {
    unsigned int operator()(const T& key, const unsigned int val) const noexcept {
        (void)key;
        return val; // We just pass back the value.
    }
};


template<class T,
         class ValueClipFn = CounterNoValueClip<T> >
class Counter {
    using N = unsigned int;
    using C = std::unordered_map<T, N>;

    C data {};
public:

    using key_type = T;

    /*
     * Modifiers
     */

    // Use this if you only ever intend to allow v>0.
    void set(const T& k, const N& v) noexcept {
        assert(v != 0);
        this->data[k] = ValueClipFn()(k, v);
    }

    // This version will allow v=0. However, it comes with branching overhead.
    void set_or_remove(const T& k, const N& v) noexcept {
        if (v) {
            this->data[k] = ValueClipFn()(k, v);
        } else {
            this->data.erase(k);
        }
    }

    // Use this if you only intend to remove existing elements.
    void remove(const T& k) noexcept {
        assert(this->get(k));
        this->try_remove(k); // We don't really have a better function for this.
    }

    // If the element doesn't exist, this won't fail.
    void try_remove(const T& k) noexcept {
        this->data.erase(k);
    }

    void merge_in(const Counter<T, ValueClipFn>& other) noexcept {
        for (const auto& e : other.data) {
            this->increment(e.first, e.second);
        }
    }

    // Do not have zeroes on the right side.
    template<class P>
    void merge_in(const P& obj) noexcept {
        for (const std::pair<T, N>& e : obj) {
            assert(e.second);
            this->increment(e.first, e.second);
        }
    }

    // Do not use this with v_to_add=0.
    void increment(const T& k, const N& v_to_add) noexcept {
        assert(v_to_add != 0);
        const auto result = this->data.find(k);
        if (result == this->data.end()) {
            this->data.emplace(k, ValueClipFn()(k, v_to_add));
        } else {
            result->second = ValueClipFn()(k, result->second + v_to_add);
        }
    }

    // Do not use this with v_to_subtract=0.
    // Do not use this with keys that aren't in the counter.
    void decrement(const T& k, const N& v_to_subtract) noexcept {
        assert(v_to_subtract != 0);
        const auto result = this->data.find(k);
        assert(result != this->data.end()); // We must have this key in the map.
        if (v_to_subtract < result->second) {
            result->second -= v_to_subtract;
        } else {
            this->data.erase(result);
        }
    }

    /*
     * Accessors
     */

    N get(const T& k) const noexcept {
        const auto result = this->data.find(k);
        if (result == this->data.end()) {
            return 0;
        } else {
            assert(result->second);
            return result->second;
        }
    }

    N sum() const noexcept {
        N ret = 0;
        for (const auto& e : this->data) {
            ret += e.second;
        }
        return ret;
    }

    std::vector<std::pair<T, N>> as_vector() const noexcept {
        std::vector<std::pair<T, N>> ret;
        for (const auto& e : this->data) {
            ret.emplace_back(e.first, e.second);
        }
        return ret;
    }

    static std::vector<Counter<T>> generate_power_set(const std::vector<T>& key_subset,
                                                      const unsigned int max_each) noexcept {
        std::vector<Counter<T>> ret {{}};
        assert(ret.size() == 1); // We start with a seeded collection.

        for (const T& key : key_subset) {
            std::vector<Counter<T>> new_ret = ret;
            for (const auto& old_counter : ret) {
                for (unsigned int v = 1; v <= max_each; ++v) {
                    new_ret.emplace_back(old_counter).set(key, v);
                }
            }
            ret = std::move(new_ret);
        }
        
        return ret;
    }

    /*
     * Direct adapted interface
     */

    auto find(const T& k) const noexcept {
        return this->data.find(k);
    }

    bool contains(const T& k) const noexcept {
        // TODO: This should eventually become a call to the actual std::unordered_map::contains()
        // method in the future when I fully migrate this codebase to C++20.
        const auto result = this->data.find(k);
        assert((result == this->data.end()) || (result->second > 0));
        return result != this->data.end();
    }

    auto begin() const noexcept {
        return this->data.begin();
    }

    auto end() const noexcept {
        return this->data.end();
    }

    auto size() const noexcept {
        return this->data.size();
    }

    bool operator==(const Counter& x) const noexcept {
        return this->data == x.data;
    }

    /*
     * Others
     */

    std::size_t calculate_hash() const noexcept {
        std::size_t ret = 0;
        for (const auto& e : this->data) {
            ret += std::hash<T>()(e.first) * std::hash<N>()(e.second);
        }
        return ret;
    }

};


// Counter specialized for pointer keys and small values (roughly 16 or less, this it remains untested).
// BEWARE: Using this template outside of its intended use cases can
//         lead to bad performance, or maybe even security issues!
template<class T,
         class ValueClipFn = CounterNoValueClip<T> >
class CounterPKSV : public Counter<T, ValueClipFn> {
    using N = unsigned int;
public:
    std::size_t calculate_hash() const noexcept {
        std::size_t ret = 0;
        for (const auto& e : *this) {
            const auto v = std::hash<T>()(e.first) << std::hash<N>()(e.second);
            assert(std::hash<T>()(e.first) >> 6); // We require a relatively large key.
            assert(v << 4); // We require a relatively comfortable buffer before we reach a zeroed register.
            ret += v;
        }
        return ret;
    }
};


} // namespace

#endif // COUNTER_H

