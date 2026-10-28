# SPDX-License-Identifier: GPL-3.0-or-later

set(QPCPP_ROOT "${PROJECT_SOURCE_DIR}/third_party/qpcpp")
if(NOT EXISTS "${QPCPP_ROOT}/include/qp.hpp")
    message(FATAL_ERROR
        "QP/C++ is missing. Run: git submodule update --init third_party/qpcpp")
endif()

# Core framework sources follow upstream's bare-metal QF source list.
# Dynamic-event support is internal framework infrastructure; Blinky itself
# uses only static storage. No QK, QXK, QS, or upstream startup code is built.
add_library(qpcpp STATIC
    "${QPCPP_ROOT}/src/qf/qep_hsm.cpp"
    "${QPCPP_ROOT}/src/qf/qep_msm.cpp"
    "${QPCPP_ROOT}/src/qf/qf_act.cpp"
    "${QPCPP_ROOT}/src/qf/qf_actq.cpp"
    "${QPCPP_ROOT}/src/qf/qf_defer.cpp"
    "${QPCPP_ROOT}/src/qf/qf_dyn.cpp"
    "${QPCPP_ROOT}/src/qf/qf_mem.cpp"
    "${QPCPP_ROOT}/src/qf/qf_ps.cpp"
    "${QPCPP_ROOT}/src/qf/qf_qact.cpp"
    "${QPCPP_ROOT}/src/qf/qf_qeq.cpp"
    "${QPCPP_ROOT}/src/qf/qf_qmact.cpp"
    "${QPCPP_ROOT}/src/qf/qf_time.cpp"
    "${QPCPP_ROOT}/src/qv/qv.cpp"
    "${QPCPP_ROOT}/ports/arm-cm/qv/gnu/qv_port.cpp"
)
target_include_directories(qpcpp PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
    "${QPCPP_ROOT}/include"
    "${QPCPP_ROOT}/ports/arm-cm/qv/gnu"
)
target_compile_features(qpcpp PUBLIC cxx_std_17)
target_link_libraries(qpcpp PUBLIC pico_platform)
