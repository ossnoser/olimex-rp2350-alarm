// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "duty_cycle.hpp"
#include "relay_test.hpp"

namespace Blinky {
void start();
// Application context only (QV/core 0). One static pending request, tagged with
// its USB generation. Model dispatch consumes/validates it before actuation.
bool requestOn(unsigned relay);
bool takeOnRequest(unsigned relay);
bool cancelPending();
void stopRelay(unsigned relay);
void stopAll();
void connectionLost();
RelayTest::State relayState();
unsigned activeRelay();
// Retained with the copied HSM for future extensions; unused by this diagnostic.
void requestDutyCycle(Commands::DutyCycle duty);
} // namespace Blinky
