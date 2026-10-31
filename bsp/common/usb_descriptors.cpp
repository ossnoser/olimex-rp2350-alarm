// SPDX-License-Identifier: GPL-3.0-or-later
#include "tusb.h"
#include "board.hpp"
#include "pico/unique_id.h"
#include <cstddef>
#include <cstdint>

namespace {
// Pico SDK development CDC VID/PID, not a product-specific USB allocation.
constexpr tusb_desc_device_t DEVICE = {
    sizeof(tusb_desc_device_t), TUSB_DESC_DEVICE, 0x0200,
    TUSB_CLASS_MISC, MISC_SUBCLASS_COMMON, MISC_PROTOCOL_IAD,
    CFG_TUD_ENDPOINT0_SIZE, 0x2e8a, 0x0009, 0x0100, 1, 2, 3, 1
};
constexpr std::uint8_t CONFIG[] = {
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN, 0, 250),
    TUD_CDC_DESCRIPTOR(0, 4, 0x81, 8, 0x02, 0x82, 64)
};
}

extern "C" std::uint8_t const *tud_descriptor_device_cb() {
    return reinterpret_cast<std::uint8_t const *>(&DEVICE);
}
extern "C" std::uint8_t const *tud_descriptor_configuration_cb(std::uint8_t) {
    return CONFIG;
}
extern "C" std::uint16_t const *tud_descriptor_string_cb(std::uint8_t index,
                                                         std::uint16_t) {
    static std::uint16_t descriptor[33];
    if (index == 0) {
        descriptor[0] = (TUSB_DESC_STRING << 8U) | 4U;
        descriptor[1] = 0x0409;
        return descriptor;
    }
    static char serial[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];
    pico_get_unique_board_id_string(serial, sizeof serial);
    char const *text;
    switch (index) {
        case 1: text = "Raspberry Pi"; break;
        case 2: text = BSP::Board::usbProductName(); break;
        case 3: text = serial; break;
        case 4: text = "Blinky Console"; break;
        default: return nullptr;
    }
    std::size_t length = 0;
    while (length < 32 && text[length] != '\0') {
        descriptor[1 + length] = static_cast<unsigned char>(text[length]);
        ++length;
    }
    descriptor[0] = static_cast<std::uint16_t>((TUSB_DESC_STRING << 8U)
                                              | (2U + length * 2U));
    return descriptor;
}
