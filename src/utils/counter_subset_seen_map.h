/*
 * File: counter_subset_seen_map.h
 * Author: <contact@simshadows.com>
 */

#ifndef COUNTER_SUBSET_SEEN_MAP_H
#define COUNTER_SUBSET_SEEN_MAP_H

#include <tuple>


namespace Utils
{

template<class Ci, class... Cv>
std::size_t do_hash(const Ci& ci, const Cv&... cv) noexcept {
    return ci.calculate_hash() + do_hash<Cv...>(cv...);
}

template<class Ci>
std::size_t do_hash(const Ci& ci) noexcept {
    return ci.calculate_hash();
}

template<class... Cv>
struct CounterTupleHash {
    using T = std::tuple<Cv...>;

    std::size_t operator()(const T& t) const noexcept {
        return std::apply(do_hash<Cv...>, t);
    }

    // The short way.
    // Also the boring way. I'm gonna try playing around with the long way for now, for fun!
    //std::size_t operator()(const T& t) const noexcept {
    //    const auto op = [](auto&... xv){
    //        return (xv.calculate_hash() + ...);
    //    };
    //    return std::apply(op, t);
    //}
};


template<class D, class... Cv>
class CounterSubsetSeenMap {
protected:
    using T = std::tuple<Cv...>;
    using H = CounterTupleHash<Cv...>;

    static_assert(std::tuple_size<T>::value == 2, "Current implementation is hardcoded to exactly two counters.");

    std::unordered_set<T, H>    seen_set {};
    std::unordered_map<T, D, H> data     {};
public:
    void add(const D& d, const Cv&... kv) {
        T k = std::make_tuple(kv...);

        if (this->seen_set.count(k)) {
            return;
        }

        this->add_power_set(k);
        this->data[k] = d;
    }

    std::vector<D> get_data_as_vector() const {
        std::vector<D> ret;
        for (const auto& e : this->data) {
            ret.emplace_back(e.second);
        }
        return ret;
    }

    std::size_t size() const noexcept {
        return this->data.size();
    }

private:
    void add_power_set(const T& k) {
        if (this->seen_set.count(k)) {
            this->data.erase(k);
            return;
        }
        this->seen_set.emplace(k);

        T new_k = k;

        // TODO: ughhhh how do I do this??????
        //for (std::size_t i = 0; i < std::tuple_size<T>::value; ++i) {

        //    const auto& curr_k_counter = std::get<i>(k);
        //    auto& curr_new_k_counter = std::get<i>(new_k);

        //    for (const auto& e : curr_k_counter) {
        //        const auto& kk = e.first;
        //        const unsigned int vv = e.second;
        //        assert(vv); // Make sure that Counter cannot produce zeroes.

        //        if (vv == 1) {
        //            curr_new_k_counter.remove(kk);
        //        } else {
        //            curr_new_k_counter.set(kk, vv - 1);
        //        }

        //        this->add_power_set(new_k);

        //        curr_new_k_counter.set(kk, vv); // Reset
        //    }
        //}


        for (const auto& e : std::get<0>(k)) {
            auto& curr_new_k_counter = std::get<0>(new_k);

            const auto& kk = e.first;
            const unsigned int vv = e.second;
            assert(vv); // Make sure that Counter cannot produce zeroes.

            if (vv == 1) {
                curr_new_k_counter.remove(kk);
            } else {
                curr_new_k_counter.set(kk, vv - 1);
            }

            this->add_power_set(new_k);

            curr_new_k_counter.set(kk, vv); // Reset
        }

        // ---

        for (const auto& e : std::get<1>(k)) {
            auto& curr_new_k_counter = std::get<1>(new_k);

            const auto& kk = e.first;
            const unsigned int vv = e.second;
            assert(vv); // Make sure that Counter cannot produce zeroes.

            if (vv == 1) {
                curr_new_k_counter.remove(kk);
            } else {
                curr_new_k_counter.set(kk, vv - 1);
            }

            this->add_power_set(new_k);

            curr_new_k_counter.set(kk, vv); // Reset
        }
    }
};


} // namespace


#endif // COUNTER_SUBSET_SEEN_MAP_H

