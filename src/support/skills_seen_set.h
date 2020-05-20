/*
 * File: skills_seen_set.h
 * Author: <contact@simshadows.com>
 *
 * The SkillsSeenSet class is being deprecated in favour of the CounterSubsetSeenMap class.
 */

#include "../core/core.h"
#include "../support/support.h"

#include "../utils/counter_subset_seen_map.h"

namespace MHWIBuildSearch
{


struct SkillsAndSetBonuses {
    SkillMap skills;
    Utils::Counter<const SetBonus*> set_bonuses;

    bool operator==(const SkillsAndSetBonuses& x) const noexcept {
        return (this->skills == x.skills) && (this->set_bonuses == x.set_bonuses);
    }
};


template<class D>
class SkillsSeenSet {
    Utils::CounterSubsetSeenMap<D, SkillMap, Utils::Counter<const SetBonus*>> actual;
public:
    void add(const SkillsAndSetBonuses& k, const D& v) {
        this->actual.add(v, k.skills, k.set_bonuses);
    }

    std::vector<D> get_data_as_vector() const {
        return this->actual.get_data_as_vector();
    }

    std::size_t size() const noexcept {
        return this->actual.size();
    }
};


} // namespace

