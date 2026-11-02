// SPDX-License-Identifier: GPL-3.0-or-later
#include "blinky.hpp"
#include "diagnostic.hpp"
#include "carrier.hpp"
#include "bsp.hpp"
#include "qp_port.hpp"
#include "qsafe.h"
#include <cstdlib>
#include <deque>
#include <iostream>
#include <string>
#include <vector>
#define CHECK(c) do { if (!(c)) { std::cerr << __LINE__ << ": " #c " at " << now << '\n'; std::exit(1); } } while (false)
namespace {
struct Idle {};
unsigned now = 0, samples = 0, services = 0;
bool connected = true, followup = false;
std::uint32_t generation = 1;
std::uint8_t inputMask = 15;
bool relayOn = false;
void (*wakeup)() = nullptr;
std::deque<char> rx;
std::string tx;
struct Change { unsigned tick; bool on; };
std::vector<Change> changes;
void drain() { try { QP::QF::run(); } catch (Idle const &) {} }
void tick() { ++now; QP::QTimeEvt::tick(0U, &now); drain(); }
}
void QP::QF::onStartup() {}
void QP::QF::onCleanup() {}
void QP::QV::onIdle() { throw Idle{}; }
extern "C" [[noreturn]] void Q_onError(char const *const module, int_t const id) {
    std::cerr << module << ':' << id << '\n'; std::abort();
}
void BSP::setLed(bool on) { changes.push_back({now, on}); }
void BSP::setSerialWakeup(void (*fn)()) { wakeup = fn; }
void BSP::serialService() {
    ++services;
    if (followup) { followup = false; wakeup(); }
}
bool BSP::serialConnected() { return connected; }
std::uint32_t BSP::serialGeneration() { return generation; }
bool BSP::serialRead(char &byte) {
    if (rx.empty()) return false;
    byte = rx.front(); rx.pop_front(); return true;
}
std::size_t BSP::serialWrite(char const *data, std::size_t count) {
    tx.append(data, count); return count;
}
Carrier::Sample Carrier::sample() { ++samples; return {now * 10U, inputMask}; }
unsigned Carrier::siliconRevision() { return 2; }
void Carrier::forceRelaysOff() { relayOn = false; }
bool Carrier::activateRelay(unsigned relay) { relayOn = relay >= 1 && relay <= 4; return relayOn; }
bool Carrier::deactivateRelay(unsigned relay) { if(relay == 1) relayOn = false; return relay >= 1 && relay <= 4; }
std::uint8_t Carrier::commandedRelays() { return relayOn ? 1U : 0U; }
int main(int argc, char **) {
    bool const offline = argc > 1;
    connected = !offline;
    QP::QF::init(); Blinky::start(); Diagnostic::start(); drain();
    CHECK(changes.size() == 1 && changes[0].tick == 0 && !changes[0].on);
    CHECK(samples == 0 && services == 1 && wakeup != nullptr);
    rx.insert(rx.end(), 100, '9');
    auto before = services;
    for (unsigned i = 0; i < 100; ++i) wakeup();
    drain(); CHECK(services == before + 1 && samples == 0 && tx.empty() && rx.size() == 100);
    followup = true; before = services;
    wakeup(); drain(); CHECK(services == before + 2 && samples == 0 && rx.size() == 100);
    for (unsigned tickNumber = 1; tickNumber <= 400; ++tickNumber) {
        inputMask = static_cast<std::uint8_t>((tickNumber / 7U) & 15U);
        tick();
        CHECK(samples == now / 2U);
        CHECK(!relayOn && Blinky::relayState() == RelayTest::State::Idle);
        if (now == 2) CHECK(rx.size() == (offline ? 100U : 84U) && tx.size() <= 64);
    }
    CHECK((offline ? rx.size() == 100U : rx.empty()) && changes.size() == 9);
    for (unsigned i = 0; i < changes.size(); ++i)
        CHECK(changes[i].tick == i * 50U && changes[i].on == ((i & 1U) != 0));
    if (offline) {
        CHECK(tx.empty()); rx.clear(); // Real BSP flushes old RX on session boundaries.
        connected = true; ++generation;
        for (unsigned i = 0; i < 20; ++i) tick();
    }
    CHECK(tx.find("Pico 2 EVB Diagnostic") != std::string::npos);
    CHECK(tx.find("raw revision=2 (A2)") != std::string::npos);
    CHECK(tx.find("Accepted") == std::string::npos);
    std::cout << "Real generated QP/QV 20 ms sampling, 50% heartbeat and coalesced USB tests passed\n";
}
