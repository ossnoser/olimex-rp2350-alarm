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
#define CHECK(c) do { if (!(c)) { std::cerr << __LINE__ << ": " #c " at " << now << '\n'; std::exit(1); } } while(false)
namespace {
struct Idle {};
unsigned now = 0, activations = 0;
bool connected = true, blocked = false, suspendResume = false;
std::uint8_t mask = 0;
std::uint32_t generation = 1;
void (*wake)() = nullptr;
std::deque<char> rx;
struct Change { unsigned tick; unsigned value; };
std::vector<Change> relay, led;
std::string tx;
void drain() { try { QP::QF::run(); } catch (Idle const &) {} }
void tick(bool dispatch = true) { ++now; QP::QTimeEvt::tick(0U, &now); if(dispatch) drain(); }
void advance(unsigned n) { while(n--) tick(); }
void feed(std::string const &s) { rx.insert(rx.end(),s.begin(),s.end()); }
void setMask(std::uint8_t next) {
    CHECK((next & (next - 1U)) == 0U); // Never two commanded coils.
    if (mask != next) relay.push_back({now,next});
    mask = next;
}
void startSequence() {
    feed("testall\n"); advance(2);
    CHECK(mask == 1 && activations == 1 && Blinky::sequenceStep() == 1);
}
void reconnect() { connected = true; ++generation; wake(); drain(); }
void locked() {
    CHECK(mask == 0 && Blinky::relayState() == RelayTest::State::Locked);
    CHECK(!Blinky::requestSequence());
}
void completeSequence() {
    auto const began = now;
    for (unsigned phase = 0; phase < 7; ++phase) {
        auto const expected = (phase % 2U) == 0 ? 1U << (phase / 2U) : 0U;
        CHECK(mask == expected && Blinky::sequenceStep() == phase / 2U + 1U);
        CHECK(Blinky::relayState() == ((phase % 2U) == 0 ? RelayTest::State::Pulsing : RelayTest::State::Gap));
        advance(99); CHECK(mask == expected);
        tick();
    }
    locked(); CHECK(activations == 4 && Blinky::sequenceStep() == 4 && relay.size() == 8);
    for (unsigned i = 0; i < 8; ++i) {
        CHECK(relay[i].tick == began + i * 100U);
        CHECK(relay[i].value == ((i % 2U) == 0 ? 1U << (i / 2U) : 0U));
    }
}
}
void QP::QF::onStartup() {}
void QP::QF::onCleanup() {}
void QP::QV::onIdle() { throw Idle{}; }
extern "C" [[noreturn]] void Q_onError(char const *const module, int_t const id) {
    std::cerr << module << ':' << id << '\n'; std::abort();
}
void BSP::setLed(bool value) { led.push_back({now,value}); }
void BSP::setSerialWakeup(void (*fn)()) { wake = fn; }
void BSP::serialService() {
    if (suspendResume) { suspendResume = false; Carrier::forceRelaysOff(); ++generation; }
}
bool BSP::serialConnected() { return connected; }
std::uint32_t BSP::serialGeneration() { return generation; }
bool BSP::serialRead(char &byte) {
    if(rx.empty()) return false;
    byte=rx.front(); rx.pop_front(); return true;
}
std::size_t BSP::serialWrite(char const *data, std::size_t count) {
    if(blocked) return 0;
    tx.append(data,count); return count;
}
Carrier::Sample Carrier::sample() { return {now*10U,static_cast<std::uint8_t>((now/7U)&15U)}; }
unsigned Carrier::siliconRevision() { return 2; }
void Carrier::forceRelaysOff() { setMask(0); }
bool Carrier::activateRelay(unsigned channel) {
    CHECK(mask == 0); // FSM must provide OFF gaps, independently of BSP clearing.
    if(channel < 1 || channel > 4) return false;
    ++activations; setMask(static_cast<std::uint8_t>(1U << (channel-1U))); return true;
}
std::uint8_t Carrier::commandedRelays() { return mask; }
int main(int argc, char **argv) {
    CHECK(argc == 2);
    std::string const mode = argv[1];
    blocked = mode == "backpressure";
    QP::QF::init(); Blinky::start(); Diagnostic::start(); drain();
    CHECK(mask == 0 && activations == 0 && Blinky::relayState() == RelayTest::State::Ready);
    if(mode == "off_ready") {
        feed("off\n"); advance(2); locked(); CHECK(activations == 0);
    } else if(mode == "pending_off" || mode == "pending_loss") {
        CHECK(Blinky::requestSequence()); CHECK(!Blinky::requestSequence());
        if(mode == "pending_off") Blinky::stopTest(); else Blinky::connectionLost();
        drain(); locked(); CHECK(activations == 0);
    } else if(mode == "stale") {
        CHECK(Blinky::requestSequence()); ++generation;
        drain(); locked(); CHECK(activations == 0);
    } else if(mode == "loss_ready") {
        connected = false; ++generation; wake(); drain();
        CHECK(mask == 0 && Blinky::relayState() == RelayTest::State::Ready);
        reconnect(); startSequence(); completeSequence();
    } else if(mode == "sequence" || mode == "backpressure") {
        startSequence();
        for(unsigned i=0;i<1000;++i) feed("testall\n"); // Stale stream cannot queue another run.
        completeSequence();
    } else if(mode == "queued_gap_timeout_off") {
        startSequence(); advance(199); CHECK(mask == 0);
        tick(false); // Gap timer is queued to start R2, but cancellation wins.
        Blinky::stopTest(); drain(); locked(); CHECK(activations == 1);
    } else {
        auto const split = mode.find("_s");
        CHECK(split != std::string::npos);
        unsigned const stage = static_cast<unsigned>(mode.back()-'0');
        CHECK(stage >= 1 && stage <= 7);
        std::string const action = mode.substr(0,split);
        startSequence(); advance((stage-1U)*100U + 20U);
        CHECK(activations == (stage+1U)/2U);
        CHECK(mask == ((stage%2U) ? 1U << ((stage-1U)/2U) : 0U));
        auto const before = activations;
        if(action == "off") { blocked=true; feed("off\n"); advance(2); }
        else if(action == "loss") {
            connected=false; ++generation; Carrier::forceRelaysOff(); wake(); drain();
        } else {
            CHECK(action == "suspend"); suspendResume=true; wake(); drain();
        }
        locked(); CHECK(activations == before);
    }
    auto const finished = activations;
    advance(300); locked(); CHECK(activations == finished && finished <= 4);
    reconnect(); locked(); feed("testall\n"); advance(20);
    CHECK(activations == finished && mask == 0);
    Blinky::stopTest(); drain(); locked(); // OFF is safe/idempotent in LOCKED.
    CHECK(led.size() == now/50U + 1U);
    for(unsigned i=0;i<led.size();++i)
        CHECK(led[i].tick == i*50U && led[i].value == (i&1U));
    std::cout << "Generated one-shot sequence policy passed: " << mode << '\n';
}
