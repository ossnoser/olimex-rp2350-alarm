// SPDX-License-Identifier: GPL-3.0-or-later
// This file is licensed under the GNU General Public License, version 3
// or (at your option) any later version.
#pragma once

namespace BSP {

inline constexpr unsigned TICKS_PER_SECOND = 100U;

// Initialize board hardware before starting the application.
// Unrecoverable failures enter the BSP fail-stop handler.
void init();

// Set the onboard LED from application context, never from an interrupt.
void setLed(bool on);

} // namespace BSP
