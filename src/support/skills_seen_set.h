/*
 * File: skills_seen_set.h
 * Author: <contact@simshadows.com>
 */

#include "../core/core.h"
#include "../support/support.h"

namespace MHWIBuildSearch
{


struct SkillsAndSetBonuses {
    SkillMap skills;
    Utils::Counter<const SetBonus*> set_bonuses;

    bool operator==(const SkillsAndSetBonuses& x) const noexcept {
        return (this->skills == x.skills) && (this->set_bonuses == x.set_bonuses);
    }
};


struct SkillsAndSetBonusesHash {
    std::size_t operator()(const SkillsAndSetBonuses& obj) const noexcept {
        std::size_t ret = obj.skills.calculate_hash();
        for (const auto& e : obj.set_bonuses) {
            ret += (std::size_t) e.first * e.second;
        }
        return ret;
    }
};


template<class AssociatedData>
class SkillsSeenSet {
    std::unordered_set<SkillsAndSetBonuses, SkillsAndSetBonusesHash> seen_set;
    std::unordered_map<SkillsAndSetBonuses, AssociatedData, SkillsAndSetBonusesHash> data;
public:
    SkillsSeenSet() noexcept = default;

    void add(SkillsAndSetBonuses& k, AssociatedData& v) {
        if (Utils::set_has_key(this->seen_set, k)) {
            return;
        }

        this->add_power_set(k);
        this->data[k] = v;
    }

    void add(SkillsAndSetBonuses&& k, AssociatedData&& v) {
        if (Utils::set_has_key(this->seen_set, k)) {
            return;
        }

        // TODO: Move k?
        this->add_power_set(k);
        this->data.emplace(std::make_pair(std::move(k), std::forward<AssociatedData>(v)));
    }

    std::vector<AssociatedData> get_data_as_vector() const {
        std::vector<AssociatedData> ret;
        for (const auto& e : this->data) {
            ret.emplace_back(e.second);
        }
        return ret;
    }

    std::size_t size() const noexcept {
        return this->data.size();
    }

private:
    void add_power_set(const SkillsAndSetBonuses& k) {
        if (Utils::set_has_key(this->seen_set, k)) {
            this->data.erase(k);
            return;
        }
        this->seen_set.emplace(k);

        SkillsAndSetBonuses new_k = k;

        for (const auto& e : k.skills) {
            const Skill * const skill = e.first;
            const unsigned int lvl = e.second;
            assert(lvl); // Skill is level 1 or greater

            if (lvl == 1) {
                new_k.skills.remove(skill);
            } else {
                assert(lvl > 1);
                new_k.skills.set(skill, lvl - 1);
            }

            this->add_power_set(new_k);

            new_k.skills.set(skill, lvl); // Reset
        }

        for (const auto& e : k.set_bonuses) {
            const SetBonus * const set_bonus = e.first;
            const unsigned int num_pieces = e.second;
            assert(num_pieces);

            if (num_pieces == 1) {
                new_k.set_bonuses.remove(set_bonus);
            } else {
                assert(num_pieces > 1);
                new_k.set_bonuses.set(set_bonus, num_pieces - 1);
            }

            this->add_power_set(new_k);

            new_k.set_bonuses.set(set_bonus, num_pieces); // Reset
        }
    }
};


} // namespace

