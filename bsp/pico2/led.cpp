// SPDX-License-Identifier: GPL-3.0-or-later
#include "bsp.hpp"
#include "board.hpp"
#include "pico.h"
#include "hardware/gpio.h"

#ifndef PICO_DEFAULT_LED_PIN
#error "Pico 2 BSP requires a directly controlled default LED GPIO."
#endif

void BSP::Board::initLed() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    // Establish the off level before enabling output to avoid an on glitch.
    gpio_put(PICO_DEFAULT_LED_PIN, false);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void BSP::setLed(bool on) {
    gpio_put(PICO_DEFAULT_LED_PIN, on);
}

char const *BSP::Board::usbProductName() {
    return "Pico 2 Blinky";
}
