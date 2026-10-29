# SPDX-License-Identifier: GPL-3.0-or-later

add_library(bsp STATIC
    pico2_w/bsp.cpp
    pico2_w/qpcpp_hooks.cpp
    pico2_w/usb_descriptors.cpp
)
target_include_directories(bsp PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}/pico2_w"
)
target_link_libraries(bsp PUBLIC
    qpcpp
    pico_stdlib
    pico_unique_id
    pico_cyw43_arch_none
    tinyusb_device
)
# The BSP owns TinyUSB. Do not let stdio initialize/service a second USB stack.
pico_enable_stdio_usb(bsp 0)
pico_enable_stdio_uart(bsp 0)
