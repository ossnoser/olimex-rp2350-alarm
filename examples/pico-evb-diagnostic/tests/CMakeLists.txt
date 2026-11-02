# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.20)
project(pico_evb_diagnostic_tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
enable_testing()
option(ENABLE_SANITIZERS "Enable address/undefined behavior sanitizers" ON)
option(ENABLE_QM_CHECKS "Run QM 5.3.0 freshness and negative tests" OFF)
if(ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()
add_compile_options(-Wall -Wextra -Wpedantic)
get_filename_component(REPO "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(APP "${CMAKE_CURRENT_LIST_DIR}/../src")
set(GENERATED "${CMAKE_CURRENT_LIST_DIR}/../generated")
set(CARRIER "${REPO}/bsp/carriers/pico_evb_rev_a")
set(QP "${REPO}/third_party/qpcpp")
add_library(report_core STATIC "${APP}/report_session.cpp")
target_include_directories(report_core PUBLIC "${APP}" "${CARRIER}/include")
add_executable(reporter_tests reporter_tests.cpp)
target_link_libraries(reporter_tests PRIVATE report_core)
add_test(NAME evb_reporter COMMAND reporter_tests)

set(QF_SOURCES qep_hsm qep_msm qf_act qf_actq qf_defer qf_dyn qf_mem
    qf_ps qf_qact qf_qeq qf_qmact qf_time)
add_library(qpcpp_host STATIC "${QP}/src/qv/qv.cpp")
foreach(source IN LISTS QF_SOURCES)
    target_sources(qpcpp_host PRIVATE "${QP}/src/qf/${source}.cpp")
endforeach()
target_include_directories(qpcpp_host PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}" "${REPO}/framework/qpcpp" "${QP}/include")

add_executable(timing_tests timing_tests.cpp
    "${APP}/blinky.cpp" "${APP}/diagnostic.cpp"
    "${GENERATED}/blinky_ao.cpp" "${GENERATED}/diagnostic_ao.cpp" "${GENERATED}/relay_test_hsm.cpp")
target_include_directories(timing_tests PRIVATE "${GENERATED}" "${REPO}/bsp/include")
target_compile_definitions(timing_tests PRIVATE QP_API_VERSION=815)
target_link_libraries(timing_tests PRIVATE report_core qpcpp_host)
add_test(NAME evb_generated_timing_usb COMMAND timing_tests)
add_test(NAME evb_startup_without_terminal COMMAND timing_tests offline)

add_executable(relay_tests relay_tests.cpp
    "${APP}/blinky.cpp" "${APP}/diagnostic.cpp"
    "${GENERATED}/blinky_ao.cpp" "${GENERATED}/diagnostic_ao.cpp" "${GENERATED}/relay_test_hsm.cpp")
target_include_directories(relay_tests PRIVATE "${GENERATED}" "${REPO}/bsp/include")
target_compile_definitions(relay_tests PRIVATE QP_API_VERSION=815)
target_link_libraries(relay_tests PRIVATE report_core qpcpp_host)
foreach(mode sequence backpressure off_ready pending_off pending_loss stale loss_ready queued_gap_timeout_off)
    add_test(NAME evb_relay_${mode} COMMAND relay_tests ${mode})
endforeach()
# Abort in each of the four pulse stages and three OFF-gap stages.
foreach(action off loss suspend)
    foreach(stage RANGE 1 7)
        add_test(NAME evb_relay_${action}_stage${stage} COMMAND relay_tests ${action}_s${stage})
    endforeach()
endforeach()

add_executable(transport_tests transport_tests.cpp "${REPO}/bsp/common/bsp.cpp"
    "${REPO}/bsp/common/runtime_options.cpp")
target_include_directories(transport_tests PRIVATE stubs "${REPO}/bsp/include"
    "${REPO}/bsp/common" "${QP}/include")
add_test(NAME evb_transport_session_loss COMMAND transport_tests)

add_executable(carrier_tests carrier_tests.cpp "${CARRIER}/carrier.cpp"
    "${REPO}/bsp/common/runtime_options.cpp" "${REPO}/bsp/common/qpcpp_hooks.cpp")
target_include_directories(carrier_tests PRIVATE stubs "${CARRIER}/include"
    "${REPO}/bsp/include" "${REPO}/bsp/common")
target_link_libraries(carrier_tests PRIVATE qpcpp_host)
add_test(NAME evb_carrier_fault_identity COMMAND carrier_tests)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME evb_model_startup_contracts COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_LIST_DIR}/contracts.py" "${CMAKE_CXX_COMPILER}")
if(ENABLE_QM_CHECKS)
    add_test(NAME evb_generated_freshness COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_LIST_DIR}/../tools/qm_codegen.py" check)
    add_test(NAME evb_codegen_negative_cases COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_LIST_DIR}/codegen_tests.py")
endif()
