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
class SSBSeenMapProto {
    using T = std::tuple<SkillMap, SetBonusMap>;
    using H = Utils::CounterTupleHash<SkillMap, SetBonusMap>;

    using T_size = std::tuple_size<T>;

    // This tree is (key_order_1.size() + key_order_2.size()) deep.
    SSBSeenTreeNode seen_tree {}; 
    // When we traverse the tree, each level corresponds to keys in this order.
    const std::vector<const Skill*>    key_order_1 {}; // The first keys are in this order.
    const std::vector<const SetBonus*> key_order_2 {}; // After key_order_1, the next keys are in this order.

    std::unordered_map<T, D, H> data {};
public:
    SSBSeenMapProto(const std::vector<const Skill*>& new_ko1, const std::vector<const SetBonus*>& new_ko2) noexcept
        : seen_tree   {}
        , key_order_1 {new_ko1}
        , key_order_2 {new_ko2}
        , data        {}
    {
        this->build_tree_1a(this->key_order_1.begin(), this->seen_tree);
    }

    SSBSeenMapProto(std::vector<const Skill*>&& new_ko1, std::vector<const SetBonus*>&& new_ko2) noexcept
        : seen_tree   {}
        , key_order_1 {std::move(new_ko1)}
        , key_order_2 {std::move(new_ko2)}
        , data        {}
    {
        this->build_tree_1a(this->key_order_1.begin(), this->seen_tree);
    }

    //void add(const D& d, const SkillMap& skills, const SetBonusMap& set_bonuses) noexcept {
    //    this->add(d, std::make_tuple(skills, set_bonuses))
    //}

    void add(const D& d, const T& k) noexcept {
        T w;
        assert(!std::get<0>(w).size());
        assert(!std::get<1>(w).size());
        const bool success = this->add_power_set_1(this->key_order_1.begin(), this->seen_tree, k, w);
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

    std::size_t key_1_maxnext(const std::vector<const Skill*>::const_iterator& p) {
        return (*p)->secret_limit;
    }

    std::size_t key_2_maxnext(const std::vector<const SetBonus*>::const_iterator& p) {
        (void)p;
        return 5; // 1 for each armour piece. This *should* be true, but idk. TODO: Make something actually robust.
    }

    void build_tree_1a(const std::vector<const Skill*>::const_iterator& p, SSBSeenTreeNode& node) {
        assert(p != this->key_order_1.end()); // We must have already checked for this condition.
        const std::size_t max_index = this->key_1_maxnext(p);
        // TODO: We could almost certainly do better than this loop below.
        node.next.reserve(max_index + 1); // TODO: Uncomment this.
        for (std::size_t i = 0; i <= max_index; ++i) {
            assert(node.next.size() == i);
            this->build_tree_1b(p, node.next.emplace_back());
        }
    }

    void build_tree_1b(const std::vector<const Skill*>::const_iterator& p, SSBSeenTreeNode& node) {
        const auto q = p + 1;
        if (q == this->key_order_1.end()) {
            // TODO: We must also deal with the case of zero keys in key_order_2.
            this->build_tree_2a(this->key_order_2.begin(), node);
        } else {
            this->build_tree_1a(q, node);
        }
    }

    void build_tree_2a(const std::vector<const SetBonus*>::const_iterator& p, SSBSeenTreeNode& node) {
        assert(p != this->key_order_2.end()); // We must have already checked for this condition.
        const std::size_t max_index = this->key_2_maxnext(p);
        // TODO: We could almost certainly do better than this loop below.
        node.next.reserve(max_index + 1); // TODO: Uncomment this.
        for (std::size_t i = 0; i <= max_index; ++i) {
            assert(node.next.size() == i);
            this->build_tree_2b(p, node.next.emplace_back());
        }
    }

    void build_tree_2b(const std::vector<const SetBonus*>::const_iterator& p, SSBSeenTreeNode& node) {
        const auto q = p + 1;
        if (q != this->key_order_2.end()) {
            this->build_tree_2a(q, node);
        }
    }

    //bool in_seen_set(const SkillMap& skills, const SetBonusMap& set_bonuses) {
    //     SSBSeenTreeNode* p = &this->seen_tree;
    //     for (const Skill * const skill : this->key_order_1) {
    //         const unsigned int lvl = skills.get(skill);
    //         assert(lvl < p->next.size());
    //         p = &(p->next[lvl]);
    //         //if (p->is_fully_populated) return true;
    //     }
    //     for (const SetBonus * const sb : this->key_order_2) {
    //         const unsigned int pieces = set_bonuses.get(sb);
    //         assert(pieces < p->next.size());
    //         p = &(p->next[pieces]);
    //         //if (p->is_fully_populated) return true;
    //     }
    //     //return false;
    //     if (p->is_fully_populated) return true;
    //}

    // Returns true if it successfully adds something.
    // (This helps cut down unnecessary writes.)
    bool add_power_set_1(const std::vector<const Skill*>::const_iterator& q, SSBSeenTreeNode& node, const T& k, T& w) {
        if (q == this->key_order_1.end()) {
            return this->add_power_set_2(this->key_order_2.begin(), node, k, w);
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

    bool add_power_set_2(const std::vector<const SetBonus*>::const_iterator& q, SSBSeenTreeNode& node, const T& k, T& w) {
        if (q == this->key_order_2.end()) {
            assert(!node.next.size());
            if (node.is_fully_populated) {
                if (k != w) {
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

