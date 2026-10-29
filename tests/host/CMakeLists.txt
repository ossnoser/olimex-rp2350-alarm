# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.20)
project(blinky_host_tests LANGUAGES CXX)
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
set(APP "${CMAKE_CURRENT_LIST_DIR}/../../examples/blinky")
set(QP "${CMAKE_CURRENT_LIST_DIR}/../../third_party/qpcpp")
add_library(console_core STATIC "${APP}/command_parser.cpp" "${APP}/console_session.cpp")
target_include_directories(console_core PUBLIC "${APP}")
add_executable(console_tests console_tests.cpp)
target_link_libraries(console_tests PRIVATE console_core)
add_test(NAME parser_console COMMAND console_tests)

# Real QP core/QV scheduler with a single-threaded host critical-section shim.
# This tests application ordering/timers, not the Arm port or USB hardware.
set(QF_SOURCES qep_hsm qep_msm qf_act qf_actq qf_defer qf_dyn qf_mem
    qf_ps qf_qact qf_qeq qf_qmact qf_time)
add_library(qpcpp_host STATIC "${QP}/src/qv/qv.cpp")
foreach(source IN LISTS QF_SOURCES)
    target_sources(qpcpp_host PRIVATE "${QP}/src/qf/${source}.cpp")
endforeach()
target_include_directories(qpcpp_host PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/../../framework/qpcpp"
    "${QP}/include"
)
add_executable(timing_tests timing_tests.cpp "${APP}/blinky.cpp" "${APP}/console.cpp")
target_include_directories(timing_tests PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../bsp/include")
target_link_libraries(timing_tests PRIVATE console_core qpcpp_host)
add_test(NAME qp_timing_ordering COMMAND timing_tests)
add_test(NAME startup_without_terminal COMMAND timing_tests disconnected)
add_test(NAME usb_service_wakeup COMMAND timing_tests usb_wakeup)
