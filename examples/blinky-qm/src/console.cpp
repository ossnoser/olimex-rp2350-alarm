// SPDX-License-Identifier: GPL-3.0-or-later
#include "console.hpp"
#include "console_ao.hpp"
#include "console_support.hpp"
#include "model_events.hpp"
#include "bsp.hpp"
#include <atomic>

namespace {
QP::QEvt const usbServiceEvent{ModelEvents::USB_SERVICE_SIG};
static_assert(std::atomic<bool>::is_always_lock_free);
std::atomic<bool> usbServiceQueued{false};

ConsoleAO &instance() {
    static ConsoleAO active;
    return active;
}

void notifyUsb() {
    if (!usbServiceQueued.exchange(true, std::memory_order_acq_rel)) {
        instance().post_(&usbServiceEvent, &usbServiceEvent);
    }
}
} // namespace

void ConsoleSupport::serviceUsb() {
    // Release before servicing so an IRQ can queue one follow-up notification.
    usbServiceQueued.store(false, std::memory_order_release);
    BSP::serialService();
}

void Console::start() {
    static QP::QEvtPtr queue[8];
    instance().start(1U, queue, 8U, nullptr, 0U);
    BSP::setSerialWakeup(&notifyUsb);
    // Kick USB work received before registration of the callback.
    notifyUsb();
}
