// SPDX-License-Identifier: GPL-3.0-or-later
#include "blinky.hpp"
#include "console.hpp"
#include "console_session.hpp"
#include "bsp.hpp"
#include "qp_port.hpp"
#include "qsafe.h"
#include <cstdlib>
#include <deque>
#include <iostream>
#include <string>
#include <vector>

#define CHECK(condition) do { if (!(condition)) { \
    std::cerr << __LINE__ << ": " #condition " at tick " << now << '\n'; std::exit(1); } } while (false)

namespace {
struct Idle {};
unsigned now = 0;
struct LedChange { unsigned tick; bool on; };
std::vector<LedChange> changes;
std::deque<char> rx;
std::string tx;
bool connected = true;
std::uint32_t generation = 1;
void (*usbWakeup)() = nullptr;
unsigned usbServiceCalls = 0;
bool wakeDuringService = false;

void drain() {
    try { QP::QF::run(); } catch (Idle const &) {}
}
void tick(bool dispatch = true) {
    ++now;
    QP::QTimeEvt::tick(0U, &now);
    if (dispatch) drain();
}
void advance(unsigned count) { while (count-- != 0) tick(); }
void feed(std::string const &text) { rx.insert(rx.end(), text.begin(), text.end()); }
void last(unsigned tickValue, bool on) {
    CHECK(!changes.empty());
    CHECK(changes.back().tick == tickValue && changes.back().on == on);
}
}

void QP::QF::onStartup() {}
void QP::QF::onCleanup() {}
void QP::QV::onIdle() { throw Idle{}; }
extern "C" [[noreturn]] void Q_onError(char const * const module, int_t const id) {
    std::cerr << "QP assertion: " << module << ':' << id << '\n';
    std::abort();
}
void BSP::init() {}
void BSP::setLed(bool on) { changes.push_back({now, on}); }
void BSP::setSerialWakeup(void (*notify)()) { usbWakeup = notify; }
void BSP::serialService() {
    ++usbServiceCalls;
    if (wakeDuringService) {
        wakeDuringService = false;
        usbWakeup();
    }
}
bool BSP::serialConnected() { return connected; }
std::uint32_t BSP::serialGeneration() { return generation; }
bool BSP::serialRead(char &byte) {
    if (rx.empty()) return false;
    byte = rx.front(); rx.pop_front(); return true;
}
std::size_t BSP::serialWrite(char const *bytes, std::size_t count) {
    tx.append(bytes, count); return count;
}

int main(int argc, char **argv) {
    bool const disconnectedTest = argc > 1 && std::string(argv[1]) == "disconnected";
    if (disconnectedTest) connected = false;
    QP::QF::init(); Blinky::start(); Console::start(); drain();
    last(0, false);
    CHECK(std::string(Console::PROMPT) == "Enter duty cycle (1..9, 10%..90% on):\r\n");
    if (argc > 1 && std::string(argv[1]) == "usb_wakeup") {
        CHECK(usbWakeup != nullptr);
        CHECK(usbServiceCalls == 1);
        feed("9\n");
        auto before = usbServiceCalls;
        for (unsigned i = 0; i < 100; ++i) usbWakeup();
        drain();
        CHECK(usbServiceCalls == before + 1);
        CHECK(rx.size() == 2 && tx.empty());
        wakeDuringService = true;
        before = usbServiceCalls;
        usbWakeup(); drain();
        CHECK(usbServiceCalls == before + 2);
        CHECK(rx.size() == 2 && tx.empty());
        rx.clear();
        advance(50); last(50, true);
        std::cout << "USB notifications coalesce and preserve polling limits\n";
        return 0;
    }
    if (disconnectedTest) {
        advance(49); last(0, false);
        advance(1); last(50, true);
        advance(50); last(100, false);
        CHECK(tx.empty());
        connected = true; ++generation; advance(2);
        CHECK(tx == Console::PROMPT);
        std::cout << "Startup without a terminal defaults to 50% duty cycle\n";
        return 0;
    }
    advance(2);
    CHECK(tx == Console::PROMPT);
    feed("1\n"); advance(2);
    CHECK(tx.find("Accepted\r\n") != std::string::npos);
    last(0, false); // Do not restart/shorten the initial 500 ms off phase.
    advance(46); last(50, true); // Newly entered on phase is 100 ms.
    feed("9\n"); advance(2); last(50, true);
    advance(7); last(50, true); // Running on phase keeps its 100 ms duration.

    tick(false); // Tick 60 queues the timeout while Console could be running.
    Blinky::requestDutyCycle({7});
    drain(); last(60, false); // New off phase is 300 ms, not 100 ms from '9'.
    advance(29); last(60, false);
    tick(false); // Tick 90 queues an off timeout.
    Blinky::requestDutyCycle({2});
    drain(); last(90, true); // New on phase is 200 ms.
    advance(20); last(110, false);

    feed("8\n3\n");
    advance(2); CHECK(!rx.empty());
    advance(2); CHECK(rx.empty());
    advance(76); last(190, true); // Old off phase (800 ms) finished unchanged.
    advance(29); last(190, true);
    advance(1); last(220, false); // Latest command '3' wins: 300 ms on.

    feed("0\n1,5\n"); advance(4);
    CHECK(tx.find("Invalid\r\n") != std::string::npos);
    feed("4"); advance(2);
    connected = false; ++generation; advance(2);
    tx.clear(); rx.clear();
    connected = true; ++generation; advance(2);
    CHECK(tx == Console::PROMPT);
    feed("0\n"); advance(2);
    CHECK(tx.find("Invalid\r\n") != std::string::npos);
    advance(58); last(290, true); // Settings survive disconnect/invalid input.

    // Exercise every immutable command event through Console and verify two
    // consecutive settled phases: on + off is exactly 100 ticks (one second).
    for (unsigned duty = 1; duty <= 9; ++duty) {
        auto previousChanges = changes.size();
        feed(std::to_string(duty) + "\n"); advance(2);
        CHECK(rx.empty());
        unsigned waited = 0;
        while (changes.size() == previousChanges && waited++ < 100) tick();
        CHECK(changes.size() == previousChanges + 1);
        unsigned const start = now;
        bool const phase = changes.back().on;
        unsigned const firstDuration = (phase ? duty : 10U - duty) * 10U;
        advance(firstDuration - 1); last(start, phase);
        tick(); last(start + firstDuration, !phase);
        advance(100U - firstDuration - 1U); last(start + firstDuration, !phase);
        tick(); last(start + 100U, phase);
    }
    CHECK(tx.find("Blink!") == std::string::npos);
    std::cout << "Real QP/QV duty-cycle timing and ordering tests passed\n";
}
