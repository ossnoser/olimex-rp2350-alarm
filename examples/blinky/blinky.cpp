// SPDX-License-Identifier: GPL-3.0-or-later
#include "blinky.hpp"
#include "bsp.hpp"
#include "qp_port.hpp"

namespace {

constexpr QP::QSignal TIMEOUT_SIG = QP::Q_USER_SIG;
constexpr unsigned HALF_SECOND_TICKS = BSP::TICKS_PER_SECOND / 2U;
static_assert(BSP::TICKS_PER_SECOND % 2U == 0U);
static_assert(HALF_SECOND_TICKS > 0U);

class BlinkyAO final : public QP::QActive {
public:
    BlinkyAO() : QP::QActive(&initial), timeout_(this, TIMEOUT_SIG) {}

private:
    QP::QTimeEvt timeout_;
    Q_STATE_DECL(initial);
    Q_STATE_DECL(off);
    Q_STATE_DECL(on);
};

Q_STATE_DEF(BlinkyAO, initial) {
    static_cast<void>(e);
    timeout_.armX(HALF_SECOND_TICKS, HALF_SECOND_TICKS);
    return tran(&off);
}

Q_STATE_DEF(BlinkyAO, off) {
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP::setLed(false);
            return Q_HANDLED();
        case TIMEOUT_SIG:
            return tran(&on);
        default:
            return super(&QP::QHsm::top);
    }
}

Q_STATE_DEF(BlinkyAO, on) {
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP::setLed(true);
            return Q_HANDLED();
        case TIMEOUT_SIG:
            return tran(&off);
        default:
            return super(&QP::QHsm::top);
    }
}

} // namespace

void Blinky::start() {
    // Static storage, constructed only after QP initialization.
    static BlinkyAO active;
    static QP::QEvtPtr queue[8];
    active.start(1U, queue, 8U, nullptr, 0U);
}
