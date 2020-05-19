/*
 * File: counter.h
 * Author: <contact@simshadows.com>
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
struct CounterNoValueClip {
    unsigned int operator()(const T& key, const unsigned int val) const noexcept {
        (void)key;
        return val; // We just pass back the value.
    }
};


template<class T,
         class ValueClipFn = CounterNoValueClip<T> >
class Counter {
protected:
    using N = unsigned int;
    using C = std::unordered_map<T, N>;

    C data;
public:

    /*
     * Modifiers
     */

    // Do not use this with v=0. Behaviour will be undefined.
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

    void remove(const T& k) noexcept {
        assert(this->get(k));
        this->data.erase(k);
    }

    void merge_in(const Counter<T, ValueClipFn>& other) noexcept {
        for (const auto& e : other.data) {
            this->increment(e.first, e.second);
        }
    }

    // Do not use this with v_to_add=0.
    void increment(const T& k, const N& v_to_add) noexcept {
        assert(v_to_add != 0);
        if (this->data.count(k)) {
            this->data[k] = ValueClipFn()(k, this->data.at(k) + v_to_add);
        } else {
            this->data[k] = ValueClipFn()(k, v_to_add);
        }
    }

    // Do not use this with v_to_subtract=0.
    // Do not use this with keys that aren't in the counter.
    void decrement(const T& k, const N& v_to_subtract) noexcept {
        assert(v_to_subtract != 0);
        assert(this->data.count(k));
        const unsigned int curr_v = this->data.at(k);
        if (v_to_subtract < curr_v) {
            this->data.at(k) = curr_v - v_to_subtract;
        } else {
            this->data.erase(k);
        }
    }

    /*
     * Accessors
     */

    N get(const T& k) const noexcept {
        // TODO: Is this the best way to do this?
        if (this->data.count(k)) {
            assert(this->data.at(k));
            return this->data.at(k);
        } else {
            return 0;
        }
    }

    /*
     * direct adapted interface
     */

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
     * others
     */

    std::size_t calculate_hash() const noexcept {
        std::size_t ret = 0;
        for (const auto& e : this->data) {
            ret += std::hash<T>()(e.first) * std::hash<N>()(e.second);
            // TODO: Maybe try playing around a bit more with this hash function.
            //ret += ((std::size_t)e.first) * e.second;
            //ret += ((std::size_t)e.first) * std::hash<N>()(e.second);
        }
        return ret;
    }

    //// I don't think I should ever need this.
    //const C& underlying() const noexcept {
    //    return this->data;
    //}

};


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

