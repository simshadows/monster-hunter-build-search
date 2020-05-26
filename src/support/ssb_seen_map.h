/*
 * File: ssb_seen_map.h
 * Author: <contact@simshadows.com>
 */

#ifndef SSB_SEEN_MAP_H
#define SSB_SEEN_MAP_H

#include <tuple>
#include "support.h"
#include "../utils/naive_counter_subset_seen_map.h"


namespace MHWIBuildSearch
{


template<class D>
class SSBSeenMapProto {
    using T = std::tuple<SkillMap, SetBonusMap>;
    using T_size = std::tuple_size<T>;

    template<std::size_t I>
    using K = typename std::tuple_element<I, T>::type::key_type;

    using O = std::tuple<std::vector<SkillMap::key_type>, std::vector<SetBonusMap::key_type>>;

    template<std::size_t I>
    using O_iter = typename std::tuple_element<I, O>::type::const_iterator;

    using H = Utils::CounterTupleHash<SkillMap, SetBonusMap>;

    // When we traverse the tree, each level corresponds to keys in this order.
    O key_order;

    std::vector<bool> seen_tree; 

    std::unordered_map<T, D, H> data;

public:

    template<class... Args>
    SSBSeenMapProto(Args&&... args) noexcept
        : key_order {std::make_tuple(std::forward<Args>(args)...)}
        , seen_tree {build_tree(key_order)}
        , data      {}
    {
    }

    void add(D&& d, T&& k) noexcept {
        T w;
        const bool success = this->add_power_set_1(std::get<0>(this->key_order).begin(), 0, this->seen_tree.size(), k, w);
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

    static std::size_t key_maxnext(const K<0>& x) {
        return x->secret_limit;
    }

    static std::size_t key_maxnext(const K<1>& x) {
        (void)x;
        return 5; // 1 for each armour piece. This *should* be true, but idk. TODO: Make something actually robust.
    }

    static std::vector<bool> build_tree(const O& new_key_order) noexcept {
        std::size_t vec_size = 1;

        const auto op1 = [&vec_size](auto& x){
            for (const auto& e : x) {
                vec_size *= (key_maxnext(e) + 1); // +1 due to having to include zero values of the counter.
            }
        };
        const auto op2 = [&op1](auto&... xv){
            return (op1(xv), ...);
        };
        std::apply(op2, new_key_order);

        return std::vector<bool>(vec_size, false);
    }

    bool add_power_set_1(const O_iter<0>& q, const std::size_t tree_lo, const std::size_t tree_hi, const T& k, T& w) {
        assert(tree_lo < tree_hi);
        if (q == std::get<0>(this->key_order).end()) {
            return this->add_power_set_2(std::get<1>(this->key_order).begin(), tree_lo, tree_hi, k, w);
        } else {
            const unsigned int max_v = key_maxnext(*q);
            const unsigned int v = std::get<0>(k).get(*q);
            assert(v <= max_v);
            // TODO: Having to use signed int here is weird. Change it?
            for (int i = v; i >= 0; --i) {
                std::get<0>(w).set_or_remove(*q, i); // TODO: Somehow use explicit set/remove?
                const std::size_t next_width = (tree_hi - tree_lo) / (max_v + 1);
                const std::size_t next_tree_lo = tree_lo + (i * next_width);
                const std::size_t next_tree_hi = next_tree_lo + next_width;
                assert(tree_lo <= next_tree_lo);
                assert(next_tree_lo <= next_tree_hi);
                assert(next_tree_hi <= tree_hi);
                const bool success = this->add_power_set_1(q + 1, next_tree_lo, next_tree_hi, k, w);
                if (!success) {
                    return ((unsigned int) i != v) && (v > 0);
                }
            }
            assert(!std::get<0>(w).contains(*q));
            return true;
        }
    }

    bool add_power_set_2(const O_iter<1>& q, const std::size_t tree_lo, const std::size_t tree_hi, const T& k, T& w) {
        assert(tree_lo < tree_hi);
        if (q == std::get<1>(this->key_order).end()) {
            return this->add_power_set_3(tree_lo, tree_hi, k, w);
        } else {
            const unsigned int max_v = key_maxnext(*q);
            const unsigned int v = std::get<1>(k).get(*q);
            assert(v <= max_v);
            // TODO: Having to use signed int here is weird. Change it?
            for (int i = v; i >= 0; --i) {
                std::get<1>(w).set_or_remove(*q, i); // TODO: Somehow use explicit set/remove?
                const std::size_t next_width = (tree_hi - tree_lo) / (max_v + 1);
                const std::size_t next_tree_lo = tree_lo + (i * next_width);
                const std::size_t next_tree_hi = next_tree_lo + next_width;
                assert(tree_lo <= next_tree_lo);
                assert(next_tree_lo < next_tree_hi);
                assert(next_tree_hi <= tree_hi);
                const bool success = this->add_power_set_2(q + 1, next_tree_lo, next_tree_hi, k, w);
                if (!success) {
                    return ((unsigned int) i != v) && (v > 0);
                }
            }
            assert(!std::get<1>(w).contains(*q));
            return true;
        }
    }

    bool add_power_set_3(const std::size_t tree_lo, const std::size_t tree_hi, const T& k, T& w) {
        (void)tree_hi;
        assert(tree_lo == tree_hi - 1); // We must have already found the element we're interested in.
        assert(tree_lo < this->seen_tree.size());
        if (this->seen_tree[tree_lo]) {
            if (k != w) {
                // We need this to avoid accidentally deleting data for an input that was already seen.
                this->data.erase(w);
            }
            return false;
        } else {
            this->seen_tree[tree_lo] = true;
            return true;
        }
    }

};


} // namespace


#endif // SSB_SEEN_MAP_H

