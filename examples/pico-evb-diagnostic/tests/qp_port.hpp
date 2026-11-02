// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include "qp_config.hpp"

// Single-threaded simulation only: not a deployable port.
#define QV_ARM_ERRATUM_838869() ((void)0)
#define QV_CPU_SLEEP() ((void)0)
#define QACTIVE_EQUEUE_TYPE QEQueue
#define QACTIVE_THREAD_TYPE void const *
#define QF_INT_DISABLE() ((void)0)
#define QF_INT_ENABLE() ((void)0)
#define QF_CRIT_STAT
#define QF_CRIT_ENTRY() ((void)0)
#define QF_CRIT_EXIT() ((void)0)
#define QF_CRIT_EST() ((void)0)
#define QF_LOG2(n_) static_cast<std::uint_fast8_t>(32U - __builtin_clz(static_cast<unsigned>(n_)))
#include "qequeue.hpp"
#include "qmpool.hpp"
#include "qp.hpp"
#include "qv.hpp"
