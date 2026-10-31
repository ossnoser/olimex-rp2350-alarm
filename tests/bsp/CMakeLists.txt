# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.20)
project(bsp_led_tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
enable_testing()
option(ENABLE_SANITIZERS "Enable address/undefined behavior sanitizers" ON)
if(ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()
add_compile_options(-Wall -Wextra -Wpedantic)
get_filename_component(REPO "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

function(add_led_test name board wireless pin)
    add_executable(${name} led_tests.cpp "${REPO}/bsp/${board}/led.cpp")
    target_include_directories(${name} PRIVATE
        stubs "${REPO}/bsp/include" "${REPO}/bsp/common"
        "${REPO}/third_party/qpcpp/include"
    )
    target_compile_definitions(${name} PRIVATE EXPECT_WIRELESS=${wireless})
    if(wireless)
        target_compile_definitions(${name} PRIVATE CYW43_WL_GPIO_LED_PIN=${pin})
    else()
        target_compile_definitions(${name} PRIVATE PICO_DEFAULT_LED_PIN=${pin})
    endif()
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_led_test(pico2_led pico2 0 25)
# Verify the implementation uses the board macro, not a hardcoded pin number.
add_led_test(pico2_led_macro_override pico2 0 7)
add_led_test(pico2_w_led pico2_w 1 0)
