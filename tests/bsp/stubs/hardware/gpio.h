// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
inline constexpr bool GPIO_OUT = true;
void gpio_init(unsigned pin);
void gpio_put(unsigned pin, bool value);
void gpio_set_dir(unsigned pin, bool output);
