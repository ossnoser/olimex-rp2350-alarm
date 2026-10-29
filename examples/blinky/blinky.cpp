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
constexpr QP::QSignal TIMING_SIG = QP::Q_USER_SIG + 1U;

struct TimingEvent : QP::QEvt {
    Commands::Timing const timing;
    constexpr TimingEvent(unsigned off, unsigned on)
        : QP::QEvt(TIMING_SIG), timing{off, on} {}
};

template<std::size_t... I>
constexpr auto makeEvents(std::index_sequence<I...>) {
    return std::array<TimingEvent, sizeof...(I)>{
        TimingEvent(static_cast<unsigned>(I / 5U + 1U),
                    static_cast<unsigned>(I % 5U + 1U))...};
}
constexpr auto timingEvents = makeEvents(std::make_index_sequence<25>{});

class BlinkyAO final : public QP::QActive {
public:
    BlinkyAO() : QP::QActive(&initial), timeout_(this, TIMEOUT_SIG) {}
private:
    QP::QTimeEvt timeout_;
    unsigned offTicks_ = BSP::TICKS_PER_SECOND / 2U;
    unsigned onTicks_ = BSP::TICKS_PER_SECOND / 2U;
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
    if (e->sig == TIMING_SIG) {
        auto const &timing = static_cast<TimingEvent const *>(e)->timing;
        offTicks_ = timing.offSeconds * BSP::TICKS_PER_SECOND;
        onTicks_ = timing.onSeconds * BSP::TICKS_PER_SECOND;
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

void Blinky::requestTiming(Commands::Timing timing) {
    Q_REQUIRE_ID(1, timing.offSeconds >= 1U && timing.offSeconds <= 5U
                    && timing.onSeconds >= 1U && timing.onSeconds <= 5U);
    unsigned const index = (timing.offSeconds - 1U) * 5U + timing.onSeconds - 1U;
    instance().postLIFO(&timingEvents[index]);
}
