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


// TODO: I need to try to come up with better names
// TODO: Also, I should probably make this a union or something.

struct SSBSeenTreeNode {
    std::vector<SSBSeenTreeNode> next {};
    bool is_fully_populated {false};
};


template<class D>
class SSBSeenMapOldProto {
    using T = std::tuple<SkillMap, SetBonusMap>;
    using H = Utils::CounterTupleHash<SkillMap, SetBonusMap>;

    using O = std::tuple<std::vector<SkillMap::key_type>, std::vector<SetBonusMap::key_type>>;

    template<std::size_t I>
    using O_iter = typename std::tuple_element<I, O>::type::const_iterator;

    using T_size = std::tuple_size<T>;

    SSBSeenTreeNode seen_tree {}; 
    // When we traverse the tree, each level corresponds to keys in this order.
    O key_order;

    std::unordered_map<T, D, H> data {};
public:

    SSBSeenMapOldProto(std::vector<const Skill*>&& new_ko1, std::vector<const SetBonus*>&& new_ko2) noexcept
        : seen_tree {}
        , key_order {std::make_tuple(std::move(new_ko1), std::move(new_ko2))}
        , data      {}
    {
        this->build_tree_1a(std::get<0>(this->key_order).begin(), this->seen_tree);
    }

    //void add(const D& d, const SkillMap& skills, const SetBonusMap& set_bonuses) noexcept {
    //    this->add(d, std::make_tuple(skills, set_bonuses))
    //}

    void add(const D& d, const T& k) noexcept {
        T w;
        assert(!std::get<0>(w).size());
        assert(!std::get<1>(w).size());
        const bool success = this->add_power_set_1(std::get<0>(this->key_order).begin(), this->seen_tree, k, w);
        if (success) {
            this->data.emplace(std::make_pair(k, d));
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

    std::size_t key_1_maxnext(const O_iter<0>& p) {
        return (*p)->secret_limit;
    }

    std::size_t key_2_maxnext(const O_iter<1>& p) {
        (void)p;
        return 5; // 1 for each armour piece. This *should* be true, but idk. TODO: Make something actually robust.
    }

    void build_tree_1a(const O_iter<0>& p, SSBSeenTreeNode& node) {
        assert(p != std::get<0>(this->key_order).end()); // We must have already checked for this condition.
        const std::size_t max_index = this->key_1_maxnext(p);
        // TODO: We could almost certainly do better than this loop below.
        node.next.reserve(max_index + 1); // TODO: Uncomment this.
        for (std::size_t i = 0; i <= max_index; ++i) {
            assert(node.next.size() == i);
            this->build_tree_1b(p, node.next.emplace_back());
        }
    }

    void build_tree_1b(const O_iter<0>& p, SSBSeenTreeNode& node) {
        const auto q = p + 1;
        if (q == std::get<0>(this->key_order).end()) {
            this->build_tree_2a(std::get<1>(this->key_order).begin(), node);
        } else {
            this->build_tree_1a(q, node);
        }
    }

    void build_tree_2a(const O_iter<1>& p, SSBSeenTreeNode& node) {
        assert(p != std::get<1>(this->key_order).end()); // We must have already checked for this condition.
        const std::size_t max_index = this->key_2_maxnext(p);
        // TODO: We could almost certainly do better than this loop below.
        node.next.reserve(max_index + 1); // TODO: Uncomment this.
        for (std::size_t i = 0; i <= max_index; ++i) {
            assert(node.next.size() == i);
            this->build_tree_2b(p, node.next.emplace_back());
        }
    }

    void build_tree_2b(const O_iter<1>& p, SSBSeenTreeNode& node) {
        const auto q = p + 1;
        if (q != std::get<1>(this->key_order).end()) {
            this->build_tree_2a(q, node);
        }
    }

    // Returns true if it successfully adds something.
    // (This helps cut down unnecessary writes.)
    bool add_power_set_1(const O_iter<0>& q, SSBSeenTreeNode& node, const T& k, T& w) {
        if (q == std::get<0>(this->key_order).end()) {
            return this->add_power_set_2(std::get<1>(this->key_order).begin(), node, k, w);
        } else {
            const unsigned int lvl = std::get<0>(k).get(*q);
            assert(node.next.size() == key_1_maxnext(q) + 1);
            assert(lvl < node.next.size());
            // TODO: Having to use signed int here is weird. Change it?
            for (int i = lvl; i >= 0; --i) {
                std::get<0>(w).set_or_remove(*q, i); // TODO: Somehow use explicit set/remove?
                SSBSeenTreeNode& next_node = node.next[i];
                const bool success = this->add_power_set_1(q + 1, next_node, k, w);
                if (!success) {
                    //std::get<0>(w).try_remove(*q);
                    return ((unsigned int) i != lvl) && (lvl > 0);
                }
            }
            assert(!std::get<0>(w).contains(*q));
            return true;
        }
    }

    bool add_power_set_2(const O_iter<1>& q, SSBSeenTreeNode& node, const T& k, T& w) {
        if (q == std::get<1>(this->key_order).end()) {
            assert(!node.next.size());
            if (node.is_fully_populated) {
                if (k != w) {
                    // We need this to avoid accidentally deleting data for an input that was already seen.
                    this->data.erase(w);
                }
                return false;
            } else {
                node.is_fully_populated = true;
                return true;
            }
        } else {
            const unsigned int pieces = std::get<1>(k).get(*q);
            assert(node.next.size() == key_2_maxnext(q) + 1);
            assert(pieces < node.next.size());
            // TODO: Having to use signed int here is weird. Change it?
            for (int i = pieces; i >= 0; --i) {
                std::get<1>(w).set_or_remove(*q, i); // TODO: Somehow use explicit set/remove?
                SSBSeenTreeNode& next_node = node.next[i];
                const bool success = this->add_power_set_2(q + 1, next_node, k, w);
                if (!success) {
                    //std::get<1>(w).try_remove(*q);
                    return ((unsigned int) i != pieces) && (pieces > 0);
                }
            }
            assert(!std::get<1>(w).contains(*q)); // We should've finished the loop at i==0, thus *q gets removed.
            return true;
        }
    }

};


} // namespace


#endif // SSB_SEEN_MAP_H

