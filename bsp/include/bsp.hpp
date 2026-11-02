// SPDX-License-Identifier: GPL-3.0-or-later
// This file is licensed under the GNU General Public License, version 3
// or (at your option) any later version.
#pragma once
#include <cstddef>
#include <cstdint>

namespace BSP {
inline constexpr unsigned TICKS_PER_SECOND = 100U;

void init();
void setLed(bool on);

// Optional startup-only configuration. Defaults preserve standalone examples.
// Register before QF/USB startup. Action must be bounded and IRQ-safe: no QP,
// allocation, waits or USB. It executes with interrupts disabled before halt.
void setFaultAction(void (*action)());
// Optional startup-only OFF action on session boundaries/suspend. Same bounded
// requirements as fault action. Defaults to disabled for standalone examples.
void setSerialLossAction(void (*action)());
// ASCII 1..32 characters, storage must outlive USB. Call only before init().
// Invalid names are rejected without replacing the current/default identity.
bool setUsbProductName(char const *name);

// USB transport only, called in application context. Never waits for a host.
// Register after the receiving AO starts. Callback may run in USB IRQ context;
// it must only notify the active object, never parse input or service USB.
void setSerialWakeup(void (*notify)());
void serialService();
bool serialConnected();
// Changes on session boundaries, including a close/reopen within one poll.
std::uint32_t serialGeneration();
bool serialRead(char &byte);
// Return the number queued, possibly zero. The caller retains unsent bytes.
std::size_t serialWrite(char const *bytes, std::size_t count);
} // namespace BSP
