# SPDX-License-Identifier: GPL-3.0-or-later

# Consume tracked generator output: a normal build does not locate/run QM.
add_executable(qpcpp-blinky-qm
    src/main.cpp
    src/blinky.cpp
    src/console.cpp
    src/command_parser.cpp
    src/console_session.cpp
    generated/blinky_ao.cpp
    generated/console_ao.cpp
)
target_include_directories(qpcpp-blinky-qm PRIVATE src generated)
target_compile_definitions(qpcpp-blinky-qm PRIVATE QP_API_VERSION=815)
target_link_libraries(qpcpp-blinky-qm PRIVATE bsp)
pico_enable_stdio_usb(qpcpp-blinky-qm 0)
pico_enable_stdio_uart(qpcpp-blinky-qm 0)
pico_add_extra_outputs(qpcpp-blinky-qm)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
# These targets are explicit and are not dependencies of either firmware.
add_custom_target(blinky-qm-regenerate
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/tools/qm_codegen.py" regenerate
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" VERBATIM
)
add_custom_target(blinky-qm-check
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/tools/qm_codegen.py" check
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" VERBATIM
)
