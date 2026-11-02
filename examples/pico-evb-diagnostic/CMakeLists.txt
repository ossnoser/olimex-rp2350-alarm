# SPDX-License-Identifier: GPL-3.0-or-later
if(NOT PICO_BOARD STREQUAL "pico2")
    message(FATAL_ERROR "PICO-EVB diagnostic supports only non-wireless pico2.")
endif()
add_executable(qpcpp-pico-evb-diagnostic
    src/main.cpp src/blinky.cpp src/diagnostic.cpp src/report_session.cpp
    generated/blinky_ao.cpp generated/diagnostic_ao.cpp generated/relay_test_hsm.cpp
)
target_include_directories(qpcpp-pico-evb-diagnostic PRIVATE src generated)
target_compile_definitions(qpcpp-pico-evb-diagnostic PRIVATE QP_API_VERSION=815)
target_link_libraries(qpcpp-pico-evb-diagnostic PRIVATE pico_evb_rev_a)
pico_enable_stdio_usb(qpcpp-pico-evb-diagnostic 0)
pico_enable_stdio_uart(qpcpp-pico-evb-diagnostic 0)
pico_add_extra_outputs(qpcpp-pico-evb-diagnostic)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
foreach(mode regenerate check)
    add_custom_target(pico-evb-diagnostic-${mode}
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/tools/qm_codegen.py" ${mode}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" VERBATIM)
endforeach()
