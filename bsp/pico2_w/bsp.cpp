// SPDX-License-Identifier: GPL-3.0-or-later
#include "bsp.hpp"
#include "pico/cyw43_arch.h"
#include "qsafe.h"

void BSP::init() {
    if (cyw43_arch_init() != 0) {
        Q_onError("cyw43_init", 1);
    }
    setLed(false);
}

void BSP::setLed(bool on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}
