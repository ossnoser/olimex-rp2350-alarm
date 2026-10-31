# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.20)

set(PICO_BOARD pico2 CACHE STRING "Supported board: pico2 or pico2_w")
set_property(CACHE PICO_BOARD PROPERTY STRINGS pico2 pico2_w)
set(PICO_PLATFORM rp2350-arm-s CACHE STRING "Supported architecture")
set(PICO_DEOPTIMIZED_DEBUG 1 CACHE BOOL "Minimize optimization in Debug builds")
if((NOT PICO_BOARD STREQUAL "pico2" AND NOT PICO_BOARD STREQUAL "pico2_w") OR
   NOT PICO_PLATFORM STREQUAL "rp2350-arm-s")
    message(FATAL_ERROR "Only pico2 or pico2_w on rp2350-arm-s is supported.")
endif()

if(NOT PICO_SDK_PATH AND DEFINED ENV{PICO_SDK_PATH})
    set(PICO_SDK_PATH "$ENV{PICO_SDK_PATH}")
endif()
if(NOT EXISTS "${PICO_SDK_PATH}/external/pico_sdk_import.cmake")
    message(FATAL_ERROR "Set PICO_SDK_PATH, for example by sourcing env.sh.")
endif()
include("${PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(qpcpp_blinky LANGUAGES C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

pico_sdk_init()
add_subdirectory(framework/qpcpp)
add_subdirectory(bsp)
add_subdirectory(examples/blinky)
add_subdirectory(examples/blinky-qm)
