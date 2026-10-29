// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>

namespace Commands {
struct Timing {
    unsigned offSeconds;
    unsigned onSeconds;
};

// Ignore ordinary ASCII spaces only; require exactly [1-5],[1-5].
// Leave result unchanged on failure.
bool parse(char const *text, std::size_t length, Timing &result);
} // namespace Commands
