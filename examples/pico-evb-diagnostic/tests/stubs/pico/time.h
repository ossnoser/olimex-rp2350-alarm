// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
struct repeating_timer_t {};
std::uint64_t time_us_64();
bool add_repeating_timer_us(std::int64_t delay, bool (*callback)(repeating_timer_t *),
                            void *data, repeating_timer_t *timer);
