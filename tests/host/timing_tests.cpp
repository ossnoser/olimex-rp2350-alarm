// SPDX-License-Identifier: GPL-3.0-or-later
#include "blinky.hpp"
#include "console.hpp"
#include "bsp.hpp"
#include "qp_port.hpp"
#include "qsafe.h"
#include <algorithm>
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
    if (argc > 1 && std::string(argv[1]) == "usb_wakeup") {
        CHECK(usbWakeup != nullptr);
        CHECK(usbServiceCalls == 1); // Kick work queued before registration.
        feed("1,5\n");
        auto before = usbServiceCalls;
        for (unsigned i = 0; i < 100; ++i) usbWakeup();
        drain();
        CHECK(usbServiceCalls == before + 1); // No event-queue overflow.
        CHECK(rx.size() == 4 && tx.empty()); // No parsing outside the poll.
        wakeDuringService = true;
        before = usbServiceCalls;
        usbWakeup(); drain();
        CHECK(usbServiceCalls == before + 2); // New work is not lost.
        CHECK(rx.size() == 4 && tx.empty());
        rx.clear();
        advance(50); last(50, true); // Normal tick/LED schedule unchanged.
        std::cout << "USB notifications coalesce and preserve polling limits\n";
        return 0;
    }
    if (disconnectedTest) {
        advance(49); last(0, false);
        advance(1); last(50, true);
        advance(50); last(100, false);
        CHECK(tx.empty());
        connected = true; ++generation; advance(2);
        CHECK(tx == "Enter seconds off,on (1..5):\r\n");
        std::cout << "Startup without a terminal uses default timings\n";
        return 0;
    }
    advance(2);
    CHECK(tx == "Enter seconds off,on (1..5):\r\n");
    feed("1,5\n"); advance(2);
    CHECK(tx.find("Accepted\r\n") != std::string::npos);
    last(0, false); // Accepted input does not restart/shorten the default phase.
    advance(46); last(50, true);
    feed("2,3\n"); advance(2); last(50, true);
    advance(497); last(50, true); // Running 5-second on phase remains unchanged.

    // Model a command posted while Console is running and Blinky's timeout
    // has become queued. The LIFO configuration event must beat that timeout.
    tick(false); // tick 550: original on phase expires.
    Blinky::requestTiming({4, 1});
    drain(); last(550, false);
    advance(399); last(550, false);
    tick(false); // tick 950: off expires; new on duration must apply now.
    Blinky::requestTiming({2, 2});
    drain(); last(950, true);
    advance(200); last(1150, false);

    // Console must yield after each command, so LIFO never reverses commands.
    feed("3,4\n1,2\n");
    advance(2); CHECK(!rx.empty());
    advance(2); CHECK(rx.empty());
    advance(196); last(1350, true);
    advance(199); last(1350, true);
    advance(1); last(1550, false); // Latest pair is 1 second off, 2 on.

    feed("0,6\n"); advance(2);
    CHECK(tx.find("Invalid\r\n") != std::string::npos);
    feed("4,"); advance(2);
    connected = false; ++generation; advance(2);
    tx.clear(); rx.clear();
    connected = true; ++generation; advance(2);
    CHECK(tx == "Enter seconds off,on (1..5):\r\n");
    feed("5\n"); advance(2);
    CHECK(tx.find("Invalid\r\n") != std::string::npos);
    advance(90); last(1650, true); // Settings survive disconnect and invalid data.
    CHECK(tx.find("Blink!") == std::string::npos);
    std::cout << "Real QP/QV timing and command-ordering tests passed\n";
}
