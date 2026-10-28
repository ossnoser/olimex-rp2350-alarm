// SPDX-License-Identifier: GPL-3.0-or-later
#include "bsp.hpp"
#include "qp_port.hpp"
#include "qsafe.h"
#include "hardware/sync.h"
#include "pico/time.h"

namespace {

repeating_timer_t tickTimer;
// Volatile stores retain the fault information for debugger inspection.
char const * volatile faultSource = nullptr;
volatile int faultIdentifier = 0;

bool tickCallback(repeating_timer_t *) {
    QP::QTimeEvt::tick(0U, &tickTimer);
    QV_ARM_ERRATUM_838869();
    return true;
}

static_assert(BSP::TICKS_PER_SECOND > 0U);
static_assert(1'000'000U % BSP::TICKS_PER_SECOND == 0U);
constexpr int64_t TICK_INTERVAL_US = 1'000'000U / BSP::TICKS_PER_SECOND;

} // namespace

void QP::QF::onStartup() {
    if (!add_repeating_timer_us(-TICK_INTERVAL_US, tickCallback,
                                nullptr, &tickTimer)) {
        Q_onError("tick_timer_start", 1);
    }
}

void QP::QF::onCleanup() {
    // Normal execution never stops the cooperative framework.
}

void QP::QV::onIdle() {
    QV_CPU_SLEEP();
}

extern "C" [[noreturn]] void Q_onError(char const * const module,
                                      int_t const id) {
    faultSource = module;
    faultIdentifier = id;
    static_cast<void>(save_and_disable_interrupts());
    for (;;) {
        __asm volatile ("nop" ::: "memory");
    }
}
