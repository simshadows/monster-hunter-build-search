/*
 * File: ssb_seen_map.h
 * Author: <contact@simshadows.com>
 */

#include "../core/core.h"
#include "../support/support.h"
#include "../utils/utils.h"
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

        // "Work key"
        // We will be reusing this object for each stage to save us
        // from copying and constructing entire tuples of counters.
        T w = k;

        for (const auto& e : std::get<0>(k)) {
            auto& w_counter = std::get<0>(w);

            const Skill * const skill = e.first;
            const unsigned int lvl = e.second;
            assert(lvl); // Skill is level 1 or greater

            if (lvl == 1) {
                w_counter.remove(skill);
            } else {
                assert(lvl > 1);
                w_counter.set(skill, lvl - 1);
            }

            this->add_power_set(w);

            w_counter.set(skill, lvl); // Reset the work key
        }

        for (const auto& e : std::get<1>(k)) {
            auto& w_counter = std::get<1>(w);

            const SetBonus * const set_bonus = e.first;
            const unsigned int num_pieces = e.second;
            assert(num_pieces); // At least one piece

            if (num_pieces == 1) {
                w_counter.remove(set_bonus);
            } else {
                assert(num_pieces > 1);
                w_counter.set(set_bonus, num_pieces - 1);
            }

            this->add_power_set(w);

            w_counter.set(set_bonus, num_pieces); // Reset the work key
        }
    }
};


} // namespace

