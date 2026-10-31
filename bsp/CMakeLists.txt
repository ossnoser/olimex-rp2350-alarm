# SPDX-License-Identifier: GPL-3.0-or-later

add_library(bsp STATIC
    common/bsp.cpp
    common/qpcpp_hooks.cpp
    common/usb_descriptors.cpp
)
target_include_directories(bsp PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}/common" # TinyUSB interface sources need tusb_config.h
)
target_link_libraries(bsp PUBLIC
    qpcpp
    pico_stdlib
    pico_unique_id
    tinyusb_device
)

if(PICO_BOARD STREQUAL "pico2")
    target_sources(bsp PRIVATE pico2/led.cpp)
    target_link_libraries(bsp PUBLIC hardware_gpio)
elseif(PICO_BOARD STREQUAL "pico2_w")
    target_sources(bsp PRIVATE pico2_w/led.cpp)
    target_link_libraries(bsp PUBLIC pico_cyw43_arch_none)
else()
    message(FATAL_ERROR "BSP supports only pico2 and pico2_w.")
endif()

# The BSP owns TinyUSB. Do not let stdio initialize/service a second USB stack.
pico_enable_stdio_usb(bsp 0)
pico_enable_stdio_uart(bsp 0)
