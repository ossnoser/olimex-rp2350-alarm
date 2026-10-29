// SPDX-License-Identifier: GPL-3.0-or-later
#include "blinky.hpp"
#include "bsp.hpp"
#include "qp_port.hpp"
#include "qsafe.h"
#include <array>
#include <utility>

namespace {
Q_DEFINE_THIS_MODULE("blinky")
constexpr QP::QSignal TIMEOUT_SIG = QP::Q_USER_SIG;
constexpr QP::QSignal DUTY_SIG = QP::Q_USER_SIG + 1U;
constexpr unsigned CYCLE_TICKS = BSP::TICKS_PER_SECOND;
static_assert(CYCLE_TICKS >= 10U && CYCLE_TICKS % 10U == 0U);

struct DutyEvent : QP::QEvt {
    Commands::DutyCycle const duty;
    constexpr explicit DutyEvent(unsigned tenths)
        : QP::QEvt(DUTY_SIG), duty{tenths} {}
};

template<std::size_t... I>
constexpr auto makeEvents(std::index_sequence<I...>) {
    return std::array<DutyEvent, sizeof...(I)>{
        DutyEvent(static_cast<unsigned>(I + 1U))...};
}
constexpr auto dutyEvents = makeEvents(std::make_index_sequence<9>{});

class BlinkyAO final : public QP::QActive {
public:
    BlinkyAO() : QP::QActive(&initial), timeout_(this, TIMEOUT_SIG) {}
private:
    QP::QTimeEvt timeout_;
    unsigned offTicks_ = CYCLE_TICKS / 2U;
    unsigned onTicks_ = CYCLE_TICKS / 2U;
    Q_STATE_DECL(initial);
    Q_STATE_DECL(running);
    Q_STATE_DECL(off);
    Q_STATE_DECL(on);
};

BlinkyAO &instance() {
    static BlinkyAO active;
    return active;
}

Q_STATE_DEF(BlinkyAO, initial) {
    static_cast<void>(e);
    static_assert(BSP::TICKS_PER_SECOND % 2U == 0U);
    return tran(&off);
}

Q_STATE_DEF(BlinkyAO, running) {
    if (e->sig == DUTY_SIG) {
        auto const tenths = static_cast<DutyEvent const *>(e)->duty.tenths;
        onTicks_ = tenths * (CYCLE_TICKS / 10U);
        offTicks_ = CYCLE_TICKS - onTicks_;
        // Do not touch the current phase's already-armed one-shot timer.
        return Q_HANDLED();
    }
    return super(&QP::QHsm::top);
}

Q_STATE_DEF(BlinkyAO, off) {
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP::setLed(false);
            timeout_.armX(offTicks_);
            return Q_HANDLED();
        case TIMEOUT_SIG:
            return tran(&on);
        default:
            return super(&running);
    }
}

Q_STATE_DEF(BlinkyAO, on) {
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP::setLed(true);
            timeout_.armX(onTicks_);
            return Q_HANDLED();
        case TIMEOUT_SIG:
            return tran(&off);
        default:
            return super(&running);
    }
}
} // namespace

void Blinky::start() {
    static QP::QEvtPtr queue[8];
    instance().start(2U, queue, 8U, nullptr, 0U);
}

void Blinky::requestDutyCycle(Commands::DutyCycle duty) {
    Q_REQUIRE_ID(1, duty.tenths >= 1U && duty.tenths <= 9U);
    instance().postLIFO(&dutyEvents[duty.tenths - 1U]);
}
