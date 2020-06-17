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


// Naive Tree Counter-Subset-Seen Map
//
// Advantages:
//      - Requires less template parameters.
//      - Default constructable.
//
// Disadvantages:
//      - Slower.
//      - Data structure can grow significantly large.
//      - Data structure is significantly more complex, leading to an expensive destructor call.
//
// Due to its simplicity, this version is suitable for simple use cases with small values.
// (This version is also a suitable model for testing more efficient implementations.)
//
template<class D, class... Cv>
class NaiveCounterSubsetSeenMap {
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
    void add(D&& d, const Cv&... kv) noexcept {
        this->add(std::move(d), std::make_tuple(kv...));
    }

    // This version is more efficient if you already bundle your counters together.
    void add(const D& d, const T& k) noexcept {
        if (Utils::map_has_key(seen_set, k)) {
            return;
        }

        this->add_power_set(k, std::make_index_sequence<T_size::value>{});
        this->data.emplace(std::make_pair(k, d));
    }
    void add(D&& d, T&& k) noexcept {
        if (Utils::map_has_key(seen_set, k)) {
            return;
        }

        this->add_power_set(k, std::make_index_sequence<T_size::value>{});
        this->data.emplace(std::make_pair(std::move(k), std::move(d)));
    }
    void add(D&& d, const T& k) noexcept {
        if (Utils::map_has_key(seen_set, k)) {
            return;
        }

        this->add_power_set(k, std::make_index_sequence<T_size::value>{});
        this->data.emplace(std::make_pair(k, std::move(d)));
    }

    std::vector<std::pair<T, D>> get_data_as_vector() const noexcept {
        std::vector<std::pair<T, D>> ret;
        for (const std::pair<T, D>& e : this->data) {
            ret.emplace_back(e);
        }
        return ret;
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

private:

    template<std::size_t... Iv>
    void add_power_set(const T& k, std::index_sequence<Iv...>) noexcept {
        if (Utils::map_has_key(seen_set, k)) {
            this->data.erase(k);
            return;
        }
        this->seen_set.emplace(k);

        // "Work key"
        // We will be reusing this object for each stage to save us
        // from copying and constructing entire tuples of counters.
        T w = k;

        (this->add_power_set_stage<Iv>(k, w), ...); // Fold
    }

    template<std::size_t I>
    void add_power_set_stage(const T& k, T& w) noexcept {
        for (const auto& e : std::get<I>(k)) {
            auto& w_counter = std::get<I>(w);

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


// Bit Tree Counter-Subset-Seen Map
//
// Advantages:
//      - Extremely space-efficient (as long as a suitable key subset and limits are used).
//      - Slightly faster.
//
// Disadvantages:
//      - Requires definition of hard limits for the values to be passed into template parameters.
//      - Constructor requires a specification of the key subset to be passed in as a parameter.
//
// Due to its simplicity, this version is suitable for simple use cases.
// (This version is also a suitable model for testing more efficient implementations.)
//
template<class D, class ValueHardLimitFn, class... Cv>
class BitTreeCounterSubsetSeenMap {
    using T = std::tuple<Cv...>;
    using T_size = std::tuple_size<T>;

    template<std::size_t I>
    using K = typename std::tuple_element<I, T>::type::key_type;

    using O = std::tuple<std::vector<typename Cv::key_type>...>;

    template<std::size_t I>
    using O_iter = typename std::tuple_element<I, O>::type::const_iterator;

    using H = Utils::CounterTupleHash<Cv...>;

    // When we traverse the tree, each level corresponds to keys in this order.
    O key_order;

    std::vector<bool> seen_tree;    // Elements are true if they're known to be seen, but not necessarily cleared.
    std::vector<bool> cleared_tree; // Elements are true if they're known to be cleared.
                                    // All cleared elements have also been seen.

    std::unordered_map<T, D, H> data;

public:

    template<class... Args>
    BitTreeCounterSubsetSeenMap(Args&&... args) noexcept
        : key_order    {std::make_tuple(std::forward<Args>(args)...)}
        , seen_tree    {build_tree(key_order)}
        , cleared_tree {build_tree(key_order)}
        , data         {}
    {
    }

    void add(D&& d, T&& k) noexcept {
        T w;
        const bool success = this->add_power_set_inode<0>(0,
                                                          this->seen_tree.size(),
                                                          k,
                                                          w,
                                                          std::get<0>(this->key_order).begin() );
        if (success) {
            this->data.emplace(std::make_pair(std::move(k), std::move(d)));
        }
    }

    std::vector<std::pair<T, D>> get_data_as_vector() const noexcept {
        std::vector<std::pair<T, D>> ret;
        for (const std::pair<T, D>& e : this->data) {
            ret.emplace_back(e);
        }
        return ret;
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

private:

    static std::vector<bool> build_tree(const O& new_key_order) noexcept {
        std::size_t vec_size = 1;

        const auto op1 = [&vec_size](auto& x){
            for (const auto& e : x) {
                vec_size *= (ValueHardLimitFn()(e) + 1); // +1 due to having to include zero values of the counter.
            }
        };
        const auto op2 = [&op1](auto&... xv){
            return (op1(xv), ...);
        };
        std::apply(op2, new_key_order);

        return std::vector<bool>(vec_size, false);
    }

    template<std::size_t I>
    bool add_power_set_inode(const std::size_t tree_lo,
                             const std::size_t tree_hi,
                             const T& k,
                             T& w,
                             const O_iter<I>& p ) {
        assert(tree_lo < tree_hi);
        if (this->cleared_tree[tree_hi - 1]) {
            return false;
        } else if (p == std::get<I>(this->key_order).end()) {
            if constexpr (I + 1 < T_size::value) {
                return this->add_power_set_inode<I + 1>(tree_lo, tree_hi, k, w, std::get<I + 1>(this->key_order).begin());
            } else {
                return this->add_power_set_leafnode(tree_lo, tree_hi, k, w);
            }
        } else {
            const unsigned int max_v = ValueHardLimitFn()(*p);
            const unsigned int v = std::get<I>(k).get(*p);
            assert(v <= max_v);
            // TODO: Having to use signed int here is weird. Change it?
            for (int i = v; i >= 0; --i) {
                std::get<I>(w).set_or_remove(*p, i); // TODO: Somehow use explicit set/remove?
                const std::size_t next_width = (tree_hi - tree_lo) / (max_v + 1);
                const std::size_t next_tree_lo = tree_lo + (i * next_width);
                const std::size_t next_tree_hi = next_tree_lo + next_width;
                assert(tree_lo <= next_tree_lo);
                assert(next_tree_lo <= next_tree_hi);
                assert(next_tree_hi <= tree_hi);
                const bool success = this->add_power_set_inode<I>(next_tree_lo, next_tree_hi, k, w, p + 1);
                if (!success) {
                    return ((unsigned int) i != v) && (v > 0);
                }
            }
            assert(!std::get<I>(w).contains(*p));
            return true;
        }
    }

    bool add_power_set_leafnode(const std::size_t tree_lo,
                                const std::size_t tree_hi,
                                const T& k,
                                T& w ) {
        (void)tree_hi;
        assert(tree_lo == tree_hi - 1); // We must have already found the element we're interested in.
        assert(tree_lo < this->seen_tree.size());
        assert(!this->cleared_tree[tree_lo]); // This was already tested.
        if (this->seen_tree[tree_lo]) {
            if (k != w) {
                // We need this to avoid accidentally deleting data for an input that was already seen.
                this->data.erase(w);
                this->cleared_tree[tree_lo] = true;
            }
            return false;
        } else {
            this->seen_tree[tree_lo] = true;
            if (k != w) {
                this->cleared_tree[tree_lo] = true;
            }
            return true;
        }
    }

};


} // namespace


#endif // COUNTER_SUBSET_SEEN_MAP_H

