// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
namespace RelayTest {
enum class State { Ready, Pulsing, Gap, Locked };
// Implemented by the owning Blinky AO adapter; no independent thread/timer owner.
void armTimeout();
void cancelTimeout();
}
