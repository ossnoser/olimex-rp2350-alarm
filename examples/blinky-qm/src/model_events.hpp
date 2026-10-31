// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "command_parser.hpp"
#include "qp_port.hpp"

namespace ModelEvents {
enum Signals : QP::QSignal {
    TIMEOUT_SIG = QP::Q_USER_SIG,
    DUTY_SIG,
    POLL_SIG,
    USB_SERVICE_SIG
};

struct DutyEvent : QP::QEvt {
    Commands::DutyCycle const duty;
    constexpr explicit DutyEvent(unsigned tenths)
        : QP::QEvt(DUTY_SIG), duty{tenths} {}
};
} // namespace ModelEvents
