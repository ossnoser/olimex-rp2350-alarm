// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
inline constexpr bool GPIO_OUT = true;
inline constexpr bool GPIO_IN = false;
void gpio_init(unsigned pin);
void gpio_put(unsigned pin, bool value);
void gpio_set_dir(unsigned pin, bool output);
void gpio_disable_pulls(unsigned pin);
void gpio_clr_mask(std::uint32_t mask);
std::uint32_t gpio_get_all();
