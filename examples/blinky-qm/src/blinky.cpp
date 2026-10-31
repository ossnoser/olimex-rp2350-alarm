// SPDX-License-Identifier: GPL-3.0-or-later
#include "blinky.hpp"
#include "blinky_ao.hpp"
#include "model_events.hpp"
#include "qsafe.h"
#include <array>
#include <utility>

namespace {
Q_DEFINE_THIS_MODULE("blinky_qm")

template<std::size_t... I>
constexpr auto makeEvents(std::index_sequence<I...>) {
    return std::array<ModelEvents::DutyEvent, sizeof...(I)>{
        ModelEvents::DutyEvent(static_cast<unsigned>(I + 1U))...};
}
constexpr auto dutyEvents = makeEvents(std::make_index_sequence<9>{});

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

void Blinky::requestDutyCycle(Commands::DutyCycle duty) {
    Q_REQUIRE_ID(1, duty.tenths >= 1U && duty.tenths <= 9U);
    instance().postLIFO(&dutyEvents[duty.tenths - 1U]);
}
