// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "command_parser.hpp"

namespace Blinky {
void start();
// Post one immutable event ahead of any queued timeout; call from Console.
void requestTiming(Commands::Timing timing);
} // namespace Blinky
