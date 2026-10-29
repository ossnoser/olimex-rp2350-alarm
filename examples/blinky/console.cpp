// SPDX-License-Identifier: GPL-3.0-or-later
#include "console.hpp"
#include "console_session.hpp"
#include "blinky.hpp"
#include "bsp.hpp"
#include "qp_port.hpp"
#include <atomic>

namespace {
constexpr QP::QSignal USB_SERVICE_SIG = QP::Q_USER_SIG + 1U;
QP::QEvt const usbServiceEvent{USB_SERVICE_SIG};
static_assert(std::atomic<bool>::is_always_lock_free);
std::atomic<bool> usbServiceQueued{false};
class UsbPort final : public Console::Port {
    void service() override { BSP::serialService(); }
    Console::Connection connection() override {
        return {BSP::serialConnected(), BSP::serialGeneration()};
    }
    bool read(char &byte) override { return BSP::serialRead(byte); }
    std::size_t write(char const *bytes, std::size_t count) override {
        return BSP::serialWrite(bytes, count);
    }
};

class ConsoleAO final : public QP::QActive {
public:
    ConsoleAO() : QP::QActive(&initial), poll_(this, QP::Q_USER_SIG) {}
private:
    Console::Session session_;
    UsbPort port_;
    QP::QTimeEvt poll_;
    Q_STATE_DECL(initial);
    Q_STATE_DECL(active);
};

ConsoleAO &instance() {
    static ConsoleAO active;
    return active;
}

void notifyUsb() {
    if (!usbServiceQueued.exchange(true, std::memory_order_acq_rel)) {
        instance().post_(&usbServiceEvent, &usbServiceEvent);
    }
}

Q_STATE_DEF(ConsoleAO, initial) {
    static_cast<void>(e);
    static_assert(BSP::TICKS_PER_SECOND % 50U == 0U);
    constexpr unsigned POLL_TICKS = BSP::TICKS_PER_SECOND / 50U;
    poll_.armX(POLL_TICKS, POLL_TICKS);
    return tran(&active);
}

Q_STATE_DEF(ConsoleAO, active) {
    if (e->sig == USB_SERVICE_SIG) {
        // Clear before servicing so new IRQ work can queue one follow-up.
        usbServiceQueued.store(false, std::memory_order_release);
        BSP::serialService();
        return Q_HANDLED();
    }
    if (e->sig == QP::Q_USER_SIG) {
        session_.poll(port_, &Blinky::requestTiming);
        return Q_HANDLED();
    }
    return super(&QP::QHsm::top);
}
} // namespace

void Console::start() {
    static QP::QEvtPtr queue[8];
    instance().start(1U, queue, 8U, nullptr, 0U);
    BSP::setSerialWakeup(&notifyUsb);
    // Also service events that arrived before callback registration.
    notifyUsb();
}
