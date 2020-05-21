/*
 * File: counter_subset_seen_map.h
 * Author: <contact@simshadows.com>
 */

#ifndef COUNTER_SUBSET_SEEN_MAP_H
#define COUNTER_SUBSET_SEEN_MAP_H

#include <tuple>


namespace Utils
{


template<class... Cv>
struct CounterTupleHash {
    using T = std::tuple<Cv...>;

    std::size_t operator()(const T& t) const noexcept {
        const auto op = [](auto&... xv){
            return (xv.calculate_hash() + ...);
        };
        return std::apply(op, t);
    }
};


template<class D, class... Cv>
class CounterSubsetSeenMap {
    using T = std::tuple<Cv...>;
    using H = CounterTupleHash<Cv...>;

    using T_size = std::tuple_size<T>;

    std::unordered_set<T, H>    seen_set {};
    std::unordered_map<T, D, H> data     {};
public:
    void add(const D& d, const Cv&... kv) noexcept {
        this->add(d, std::make_tuple(kv...));
    }
    void add(D&& d, Cv&&... kv) noexcept {
        this->add(std::move(d), std::make_tuple(std::move(kv)...));
    }

    // This version is more efficient if you already bundle your counters together.
    void add(const D& d, const T& k) noexcept {
        if (this->seen_set.count(k)) {
            return;
        }

        this->add_power_set(k, std::make_index_sequence<T_size::value>{});
        this->data[k] = d;
    }
    void add(D&& d, T&& k) noexcept {
        if (this->seen_set.count(k)) {
            return;
        }

        this->add_power_set(k, std::make_index_sequence<T_size::value>{});
        this->data[std::move(k)] = std::move(d);
    }

    std::vector<D> get_data_as_vector() const noexcept {
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

    template<std::size_t... Iv>
    void add_power_set(const T& k, std::index_sequence<Iv...>) noexcept {
        if (this->seen_set.count(k)) {
            this->data.erase(k);
            return;
        }
        this->seen_set.emplace(k);

        // "Work key"
        // We will be reusing this object for each stage to save us
        // from copying and constructing entire tuples of counters.
        T w = k;

        (this->add_power_set_stage<Iv>(k, w, std::make_index_sequence<T_size::value>{}), ...); // Fold
    }

    template<std::size_t J, std::size_t... Iv>
    void add_power_set_stage(const T& k, T& w, std::index_sequence<Iv...>) noexcept {
        for (const auto& e : std::get<J>(k)) {
            auto& w_counter = std::get<J>(w);

            const auto& kk = e.first;
            const unsigned int vv = e.second;
            assert(vv); // Make sure that Counter cannot produce zeroes.

            if (vv == 1) {
                w_counter.remove(kk);
            } else {
                w_counter.set(kk, vv - 1);
            }

            this->add_power_set(w, std::make_index_sequence<T_size::value>{});

            // We reset the work key
            w_counter.set(kk, vv);
        }
    }
};


} // namespace


#endif // COUNTER_SUBSET_SEEN_MAP_H

