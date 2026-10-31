// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>

namespace Commands {
struct DutyCycle {
    unsigned tenths; // 1..9 means 10%..90% LED-on time.
};

// Ignore ordinary ASCII spaces only; require exactly one digit [1-9].
// Leave result unchanged on failure.
bool parse(char const *text, std::size_t length, DutyCycle &result);
} // namespace Commands
