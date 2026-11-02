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
constexpr ModelEvents::RelayEvent onEvents[] = {
    {ModelEvents::RELAY_ON_SIG, 1U}, {ModelEvents::RELAY_ON_SIG, 2U},
    {ModelEvents::RELAY_ON_SIG, 3U}, {ModelEvents::RELAY_ON_SIG, 4U}};
constexpr ModelEvents::RelayEvent selectedOffEvents[] = {
    {ModelEvents::RELAY_OFF_SIG, 1U}, {ModelEvents::RELAY_OFF_SIG, 2U},
    {ModelEvents::RELAY_OFF_SIG, 3U}, {ModelEvents::RELAY_OFF_SIG, 4U}};
QP::QEvt const offEvent{ModelEvents::FORCE_OFF_SIG};
QP::QEvt const lossEvent{ModelEvents::SESSION_LOST_SIG};
bool pending = false;
std::uint32_t requestGeneration = 0;
unsigned requestedRelay = 0;

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

bool Blinky::requestOn(unsigned relay) {
    if (relay < 1U || relay > 4U || pending
        || instance().relayState() != RelayTest::State::Idle
        || Carrier::commandedRelays() != 0U || !BSP::serialConnected()) return false;
    requestGeneration = BSP::serialGeneration();
    requestedRelay = relay;
    pending = true;
    instance().post_(&onEvents[relay - 1U], &onEvents[relay - 1U]);
    return true;
}
bool Blinky::takeOnRequest(unsigned relay) {
    if (!pending || requestedRelay != relay) return false;
    bool const valid = BSP::serialConnected()
        && requestGeneration == BSP::serialGeneration()
        && instance().relayState() == RelayTest::State::Idle
        && Carrier::commandedRelays() == 0U;
    pending = false;
    return valid;
}
bool Blinky::cancelPending() {
    bool const wasPending = pending;
    pending = false;
    return wasPending;
}
void Blinky::stopRelay(unsigned relay) {
    Q_REQUIRE_ID(2, relay >= 1U && relay <= 4U);
    static_cast<void>(Carrier::deactivateRelay(relay));
    if (pending && requestedRelay == relay) static_cast<void>(cancelPending());
    instance().postLIFO(&selectedOffEvents[relay - 1U]);
}
void Blinky::stopAll() {
    Carrier::forceRelaysOff();
    static_cast<void>(cancelPending());
    instance().postLIFO(&offEvent);
}
void Blinky::connectionLost() {
    Carrier::forceRelaysOff();
    static_cast<void>(cancelPending());
    instance().postLIFO(&lossEvent);
}
RelayTest::State Blinky::relayState() { return instance().relayState(); }
unsigned Blinky::activeRelay() { return instance().activeRelay(); }

void Blinky::requestDutyCycle(Commands::DutyCycle duty) {
    Q_REQUIRE_ID(1, duty.tenths >= 1U && duty.tenths <= 9U);
    instance().postLIFO(&dutyEvents[duty.tenths - 1U]);
}
