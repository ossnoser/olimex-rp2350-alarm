// SPDX-License-Identifier: GPL-3.0-or-later
#include "bsp.hpp"
#include "board.hpp"
#include "pico/cyw43_arch.h"
#include "qsafe.h"

void BSP::Board::initLed() {
    if (cyw43_arch_init() != 0) {
        Q_onError("cyw43_init", 1);
    }
}

void BSP::setLed(bool on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

char const *BSP::Board::usbProductName() {
    return "Pico 2 W Blinky";
}
