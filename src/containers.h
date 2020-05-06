/*
 * File: containers.h
 * Author: <contact@simshadows.com>
 */

#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <unordered_map>

#include "database/database.h"

namespace MHWIBuildSearch
{


// Note that this container automatically clips levels to secret_limit.
class SkillMap {
    std::unordered_map<const Database::Skill*, unsigned int> data;
public:
    SkillMap() noexcept;

    void set_lvl(const Database::Skill* skill, unsigned int level);
    void increment_lvl(const Database::Skill* skill, unsigned int level_to_add);

    // Gets a skill's level. Skills that aren't in the container return zero.
    unsigned int get_lvl(const Database::Skill*) const;

    std::string get_humanreadable() const;
};


} // namespace

#endif // CONTAINERS_H

