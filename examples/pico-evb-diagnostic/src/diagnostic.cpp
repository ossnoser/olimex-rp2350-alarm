// SPDX-License-Identifier: GPL-3.0-or-later
#include "diagnostic.hpp"
#include "diagnostic_ao.hpp"
#include "diagnostic_support.hpp"
#include "model_events.hpp"
#include "bsp.hpp"
#include "blinky.hpp"
#include <atomic>

namespace {
QP::QEvt const usbServiceEvent{ModelEvents::USB_SERVICE_SIG};
static_assert(std::atomic<bool>::is_always_lock_free);
std::atomic<bool> usbServiceQueued{false};
bool linkSeen = false;
std::uint32_t lastGeneration = 0;
DiagnosticAO &instance() {
    static DiagnosticAO active;
    return active;
}
void notifyUsb() {
    if (!usbServiceQueued.exchange(true, std::memory_order_acq_rel)) {
        instance().post_(&usbServiceEvent, &usbServiceEvent);
    }
}
}
void DiagnosticSupport::serviceTransport() {
    BSP::serialService();
    auto const generation = BSP::serialGeneration();
    bool const connected = BSP::serialConnected();
    if (linkSeen && (!connected || generation != lastGeneration)) {
        Blinky::connectionLost();
    }
    linkSeen = connected;
    lastGeneration = generation;
}
void DiagnosticSupport::serviceUsb() {
    usbServiceQueued.store(false, std::memory_order_release);
    serviceTransport();
}
void Diagnostic::start() {
    static QP::QEvtPtr queue[8];
    instance().start(1U, queue, 8U, nullptr, 0U);
    BSP::setSerialWakeup(&notifyUsb);
    notifyUsb();
}
