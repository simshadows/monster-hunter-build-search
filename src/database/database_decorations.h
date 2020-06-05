
/*
 * This file is auto-generated.
 * Do not edit directly!
 */

#ifndef DECORATIONS_DATABASE_H
#define DECORATIONS_DATABASE_H

#include <string>
#include <vector>

#include "../core/core.h"

namespace DecorationsDatabase
{

using MHWIBuildSearch::Decoration;
using MHWIBuildSearch::Skill;

extern const std::array<const Decoration*, 400> g_all_decorations;

const Decoration* get_decoration(const std::string& deco_id) noexcept;

} // namespace

#endif // DECORATIONS_DATABASE_H

