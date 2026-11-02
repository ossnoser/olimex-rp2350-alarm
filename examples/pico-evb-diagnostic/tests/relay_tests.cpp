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
unsigned now = 0, activations = 0, samples = 0;
bool connected = true, blocked = false, suspendResume = false;
std::uint8_t mask = 0;
std::uint32_t generation = 1;
void (*wake)() = nullptr;
std::deque<char> rx;
struct Change { unsigned tick; unsigned value; };
std::vector<Change> relay, led;
std::string tx;
void drain() { try { QP::QF::run(); } catch (Idle const &) {} }
void tick() { ++now; QP::QTimeEvt::tick(0U, &now); drain(); CHECK(samples == now/2U); }
void advance(unsigned n) { while(n--) tick(); }
void feed(std::string const &s) { rx.insert(rx.end(),s.begin(),s.end()); }
void command(std::string const &s) { feed(s+"\n"); advance(2); }
void setMask(std::uint8_t next) {
    CHECK((next & (next - 1U)) == 0U); // Never two commanded coils.
    if (mask != next) relay.push_back({now,next});
    mask = next;
}
void idle() { CHECK(mask == 0 && Blinky::relayState() == RelayTest::State::Idle && Blinky::activeRelay() == 0); }
void on(unsigned channel) {
    CHECK(mask == (1U << (channel-1U)));
    CHECK(Blinky::relayState() == RelayTest::State::On && Blinky::activeRelay() == channel);
}
void startOn(unsigned channel) { command("on "+std::to_string(channel)); on(channel); }
void reconnect() { rx.clear(); connected = true; ++generation; wake(); drain(); }
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
    if (suspendResume) { suspendResume = false; Carrier::forceRelaysOff(); rx.clear(); ++generation; }
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
Carrier::Sample Carrier::sample() { ++samples; return {now*10U,static_cast<std::uint8_t>((now/7U)&15U)}; }
unsigned Carrier::siliconRevision() { return 2; }
void Carrier::forceRelaysOff() { setMask(0); }
bool Carrier::activateRelay(unsigned channel) {
    CHECK(mask == 0);
    if(channel < 1 || channel > 4) return false;
    ++activations; setMask(static_cast<std::uint8_t>(1U << (channel-1U))); return true;
}
bool Carrier::deactivateRelay(unsigned channel) {
    if(channel < 1 || channel > 4) return false;
    setMask(static_cast<std::uint8_t>(mask & ~(1U << (channel-1U)))); return true;
}
std::uint8_t Carrier::commandedRelays() { return mask; }
int main(int argc, char **argv) {
    CHECK(argc == 2 || argc == 3);
    std::string const mode = argv[1];
    unsigned const channel = argc == 3 ? static_cast<unsigned>(std::atoi(argv[2])) : 1U;
    CHECK(channel >= 1 && channel <= 4);
    unsigned const other = channel%4U+1U;
    QP::QF::init(); Blinky::start(); Diagnostic::start(); drain();
    idle(); CHECK(activations == 0);
    CHECK(!Blinky::requestOn(0) && !Blinky::requestOn(5));
    if(mode == "off_idle") {
        for(unsigned n=1;n<=4;++n) { Blinky::stopRelay(n); drain(); idle(); }
        Blinky::stopAll(); drain(); Blinky::connectionLost(); drain(); idle();
        connected=false; CHECK(!Blinky::requestOn(1)); reconnect();
    } else if(mode == "stale_channel") {
        CHECK(Blinky::requestOn(1)); Blinky::stopRelay(1);
        CHECK(Blinky::requestOn(2)); // Old channel-1 notification must not consume it.
        drain(); on(2); CHECK(activations == 1);
        CHECK(!Blinky::takeOnRequest(1)); on(2); // Late stale event cannot turn R2 off.
        Blinky::stopAll(); drain(); idle();
    } else if(mode.rfind("pending_",0) == 0) {
        CHECK(Blinky::requestOn(channel)); CHECK(!Blinky::requestOn(other));
        if(mode == "pending_same") Blinky::stopRelay(channel);
        else if(mode == "pending_other") Blinky::stopRelay(other);
        else if(mode == "pending_global") Blinky::stopAll();
        else if(mode == "pending_loss") Blinky::connectionLost();
        else { CHECK(mode == "pending_stale"); ++generation; }
        drain();
        if(mode == "pending_other") { on(channel); CHECK(activations == 1); Blinky::stopAll(); drain(); }
        else CHECK(activations == 0);
        idle(); advance(4); // Settle any diagnostic generation observation.
    } else {
        startOn(channel); auto const started = activations;
        if(mode == "hold" || mode == "flood") {
            for(unsigned n=1;n<=4;++n) {
                command("on "+std::to_string(n)); on(channel); CHECK(activations == started);
                if(n != channel) { command("off "+std::to_string(n)); on(channel); }
            }
            for(auto const *bad : {"testall","test1","on 0","on 5","on 01","ON 1","off 0","off 5","off 01","off 1x"}) {
                command(bad); on(channel); CHECK(activations == started);
            }
            if(mode == "flood") {
                blocked=true;
                for(unsigned i=0;i<1000;++i) feed("on "+std::to_string(other)+"\n");
            }
            advance(2000); on(channel); // Twenty seconds: no former 1s/7s automatic OFF.
            CHECK(rx.empty() && activations == started);
            blocked=true; command("off "+std::to_string(channel)); idle();
        } else if(mode == "global") {
            blocked=true; command("off"); idle(); // OFF never waits on TX space.
        } else if(mode == "loss") {
            connected=false; ++generation; rx.clear(); Carrier::forceRelaysOff(); wake(); drain(); idle();
            CHECK(!Blinky::requestOn(channel));
        } else if(mode == "suspend") {
            suspendResume=true; wake(); drain(); idle();
        } else {
            CHECK(mode == "generation"); ++generation; rx.clear(); Carrier::forceRelaysOff(); wake(); drain(); idle();
        }
    }
    auto const finished = activations;
    advance(800); idle(); CHECK(activations == finished); // No queued activation/sequence.
    reconnect(); advance(4); idle(); CHECK(activations == finished);
    // Fresh explicit commands work repeatedly, across channels, without reset.
    rx.clear();
    for(unsigned n=1;n<=4;++n) {
        startOn(n); advance(20); on(n);
        command("off "+std::to_string(n)); idle();
        command("off "+std::to_string(n)); idle();
    }
    Blinky::stopAll(); drain(); idle();
    CHECK(activations == finished+4U);
    CHECK(led.size() == now/50U+1U);
    for(unsigned i=0;i<led.size();++i) CHECK(led[i].tick == i*50U && led[i].value == (i&1U));
    std::cout << "Generated manual relay policy passed: " << mode << " channel " << channel << '\n';
}
