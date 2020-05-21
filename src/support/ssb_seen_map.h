/*
 * File: ssb_seen_map.h
 * Author: <contact@simshadows.com>
 */

#include "../core/core.h"
#include "../support/support.h"
#include "../utils/counter_subset_seen_map.h"

namespace MHWIBuildSearch
{


//// Template Implementation
//// TODO: Figure out why it runs slower than the specialized implementation.
//template<class StoredData>
//using SSBSeenMap = Utils::CounterSubsetSeenMap<StoredData, SkillMap, SetBonusMap>;


// Specialized Implementation
template<class StoredData>
class SSBSeenMap {
    using T = std::tuple<SkillMap, SetBonusMap>;
    using H = Utils::CounterTupleHash<SkillMap, SetBonusMap>;

    std::unordered_set<T, H> seen_set;
    std::unordered_map<T, StoredData, H> data;
public:
    void add(const StoredData& v, const T& k) {
        if (Utils::set_has_key(this->seen_set, k)) {
            return;
        }

        this->add_power_set(k);
        this->data.emplace(std::make_pair(k, v));
    }

    void add(StoredData&& v, T&& k) {
        if (Utils::set_has_key(this->seen_set, k)) {
            return;
        }

        // TODO: Move k?
        this->add_power_set(k);
        this->data.emplace(std::make_pair(std::move(k), std::move(v)));
    }

    std::vector<std::pair<T, StoredData>> get_data_as_vector() const noexcept {
        std::vector<std::pair<T, StoredData>> ret;
        for (const std::pair<T, StoredData>& e : this->data) {
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
    void add_power_set(const T& k) {
        if (Utils::set_has_key(this->seen_set, k)) {
            this->data.erase(k);
            return;
        }
        this->seen_set.emplace(k);

        T new_k = k;

        for (const auto& e : std::get<0>(k)) {
            const Skill * const skill = e.first;
            const unsigned int lvl = e.second;
            assert(lvl); // Skill is level 1 or greater

            if (lvl == 1) {
                std::get<0>(new_k).remove(skill);
            } else {
                assert(lvl > 1);
                std::get<0>(new_k).set(skill, lvl - 1);
            }

            this->add_power_set(new_k);

            std::get<0>(new_k).set(skill, lvl); // Reset
        }

        for (const auto& e : std::get<1>(k)) {
            const SetBonus * const set_bonus = e.first;
            const unsigned int num_pieces = e.second;
            assert(num_pieces);

            if (num_pieces == 1) {
                std::get<1>(new_k).remove(set_bonus);
            } else {
                assert(num_pieces > 1);
                std::get<1>(new_k).set(set_bonus, num_pieces - 1);
            }

            this->add_power_set(new_k);

            std::get<1>(new_k).set(set_bonus, num_pieces); // Reset
        }
    }
};


} // namespace

