// SPDX-License-Identifier: GPL-3.0-or-later
#include "blinky.hpp"
#include "blinky_ao.hpp"
#include "model_events.hpp"
#include "qsafe.h"
#include "bsp.hpp"
#include "carrier.hpp"
#include <array>
#include <utility>

namespace {
Q_DEFINE_THIS_MODULE("evb_blinky")

template<std::size_t... I>
constexpr auto makeEvents(std::index_sequence<I...>) {
    return std::array<ModelEvents::DutyEvent, sizeof...(I)>{
        ModelEvents::DutyEvent(static_cast<unsigned>(I + 1U))...};
}
constexpr auto dutyEvents = makeEvents(std::make_index_sequence<9>{});
QP::QEvt const testEvent{ModelEvents::TESTALL_SIG};
QP::QEvt const offEvent{ModelEvents::FORCE_OFF_SIG};
QP::QEvt const lossEvent{ModelEvents::SESSION_LOST_SIG};
bool pending = false;
std::uint32_t requestGeneration = 0;

BlinkyAO &instance() {
    // Construct only after framework initialization, as in the original.
    static BlinkyAO active;
    return active;
}
} // namespace

void Blinky::start() {
    static QP::QEvtPtr queue[8];
    instance().start(2U, queue, 8U, nullptr, 0U);
}

bool Blinky::requestSequence() {
    if (pending || instance().relayState() != RelayTest::State::Ready
        || !BSP::serialConnected()) return false;
    requestGeneration = BSP::serialGeneration();
    pending = true;
    instance().post_(&testEvent, &testEvent);
    return true;
}
bool Blinky::takeSequenceRequest() {
    bool const valid = pending && BSP::serialConnected()
        && requestGeneration == BSP::serialGeneration();
    pending = false;
    return valid;
}
bool Blinky::cancelPending() {
    bool const wasPending = pending;
    pending = false;
    return wasPending;
}
void Blinky::stopTest() {
    Carrier::forceRelaysOff();
    instance().postLIFO(&offEvent);
}
void Blinky::connectionLost() {
    Carrier::forceRelaysOff();
    instance().postLIFO(&lossEvent);
}
RelayTest::State Blinky::relayState() { return instance().relayState(); }
unsigned Blinky::sequenceStep() { return instance().sequenceStep(); }
void RelayTest::armTimeout() { instance().armRelayTimer(); }
void RelayTest::cancelTimeout() { instance().cancelRelayTimer(); }

void Blinky::requestDutyCycle(Commands::DutyCycle duty) {
    Q_REQUIRE_ID(1, duty.tenths >= 1U && duty.tenths <= 9U);
    instance().postLIFO(&dutyEvents[duty.tenths - 1U]);
}
