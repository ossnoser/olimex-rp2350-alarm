// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>

namespace Carrier {
struct Sample {
    std::uint64_t milliseconds;
    // Bits 0..3 represent raw IN1..IN4: 1 = HIGH/inactive, 0 = LOW/active.
    std::uint8_t highMask;
};
void initRelaysOff();
void forceRelaysOff();
// Checked logical relay number 1..4, NOT GPIO number. Clears all other outputs.
// Caller enforces explicit-session/one-shot sequencing. Invalid numbers fail OFF.
bool activateRelay(unsigned relay);
std::uint8_t commandedRelays(); // Bits 0..3, NOT contact/coil feedback.
void initInputs();
Sample sample();
unsigned siliconRevision();
}
