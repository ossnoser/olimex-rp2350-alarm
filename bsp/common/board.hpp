// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// Internal BSP contract, not part of the application-facing interface.
namespace BSP::Board {
void initLed();
char const *usbProductName();
} // namespace BSP::Board
