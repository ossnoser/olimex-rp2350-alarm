# SPDX-License-Identifier: GPL-3.0-or-later

add_library(bsp STATIC
    pico2_w/bsp.cpp
    pico2_w/qpcpp_hooks.cpp
)
target_include_directories(bsp PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(bsp PUBLIC
    qpcpp
    pico_stdlib
    pico_cyw43_arch_none
)
