// SPDX-License-Identifier: GPL-3.0-or-later
#include "carrier.hpp"
#include "bsp.hpp"
#include "board.hpp"
#include "runtime_options.hpp"
#include "qp_port.hpp"
#include "qsafe.h"
#include "pico/time.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#define CHECK(c) do { if (!(c)) { std::cerr << __LINE__ << ": " #c "\n"; std::exit(1); } } while (false)
namespace {
constexpr std::uint32_t RELAYS = 0x3c00U;
std::uint32_t output = ~0U, enabled = 0, levels = 0;
std::vector<std::string> calls;
unsigned revision = 2;
bool irqDisabled = false, timerFail = false;
struct Halt {};
void trap() {
    CHECK(irqDisabled);
    Carrier::forceRelaysOff();
    throw Halt{}; // Host-only escape from the real, otherwise infinite halt path.
}
}
void gpio_init(unsigned pin) {
    calls.push_back("init" + std::to_string(pin));
    enabled &= ~(1U << pin); output &= ~(1U << pin);
}
void gpio_put(unsigned pin, bool value) {
    if (value) {
        CHECK(pin >= 10U && pin <= 13U); output |= 1U << pin;
    } else output &= ~(1U << pin);
    calls.push_back(std::string(value ? "high" : "low") + std::to_string(pin));
}
void gpio_set_dir(unsigned pin, bool out) {
    calls.push_back(std::string(out ? "out" : "in") + std::to_string(pin));
    if (out) { CHECK(!(output & (1U << pin))); enabled |= 1U << pin; }
    else enabled &= ~(1U << pin);
}
void gpio_disable_pulls(unsigned pin) { calls.push_back("pulls_off" + std::to_string(pin)); }
void gpio_clr_mask(std::uint32_t mask) {
    CHECK(mask != 0 && (mask & ~RELAYS) == 0);
    calls.push_back(mask == RELAYS ? "clear_relays" : "clear_selected"); output &= ~mask;
}
std::uint32_t gpio_get_all() { return levels; }
std::uint64_t time_us_64() { return 1234567; }
unsigned rp2350_chip_version() { return revision; }
std::uint32_t save_and_disable_interrupts() { irqDisabled = true; return 0; }
bool add_repeating_timer_us(std::int64_t delay, bool (*callback)(repeating_timer_t *),
                            void *, repeating_timer_t *timer) {
    CHECK(delay == -10000);
    if (timerFail) return false;
    CHECK(callback(timer)); return true;
}
char const *BSP::Board::usbProductName() { return "Default Test Board"; }

int main() {
    BSP::Detail::invokeFaultAction(); CHECK(calls.empty());
    CHECK(std::strcmp(BSP::Detail::usbProductName(), "Default Test Board") == 0);
    CHECK(!BSP::setUsbProductName(nullptr));
    CHECK(!BSP::setUsbProductName(""));
    CHECK(!BSP::setUsbProductName("123456789012345678901234567890123"));
    CHECK(!BSP::setUsbProductName("bad\nname"));
    CHECK(std::strcmp(BSP::Detail::usbProductName(), "Default Test Board") == 0);
    CHECK(BSP::setUsbProductName("Pico 2 EVB Diagnostic"));
    CHECK(std::strcmp(BSP::Detail::usbProductName(), "Pico 2 EVB Diagnostic") == 0);

    Carrier::initRelaysOff();
    std::vector<std::string> expected{"clear_relays"};
    for (unsigned pin : {10U, 11U, 12U, 13U}) {
        expected.push_back("init" + std::to_string(pin));
        expected.push_back("low" + std::to_string(pin));
        expected.push_back("out" + std::to_string(pin));
    }
    CHECK(calls == expected && enabled == RELAYS && output == ~RELAYS);
    calls.clear();
    Carrier::initInputs(); expected.clear();
    for (unsigned pin : {2U, 3U, 8U, 9U}) {
        expected.push_back("init" + std::to_string(pin));
        expected.push_back("in" + std::to_string(pin));
        expected.push_back("pulls_off" + std::to_string(pin));
    }
    CHECK(calls == expected && enabled == RELAYS);
    unsigned const pins[] = {2, 3, 8, 9};
    for (unsigned mask = 0; mask < 16; ++mask) {
        levels = 0;
        for (unsigned i = 0; i < 4; ++i) if (mask & (1U << i)) levels |= 1U << pins[i];
        auto const sample = Carrier::sample();
        CHECK(sample.milliseconds == 1234 && sample.highMask == mask);
    }
    for (revision = 2; revision <= 4; ++revision) CHECK(Carrier::siliconRevision() == revision);
    output = ~0U; calls.clear();
    BSP::setFaultAction(&Carrier::forceRelaysOff);
    BSP::Detail::invokeFaultAction();
    CHECK(output == ~RELAYS && calls == std::vector<std::string>{"clear_relays"});
    BSP::setFaultAction(nullptr); calls.clear();
    BSP::Detail::invokeFaultAction(); CHECK(calls.empty());

    // Every checked logical relay maps correctly, with all others OFF first.
    for (unsigned relay = 1; relay <= 4; ++relay) {
        calls.clear(); output = ~0U;
        CHECK(Carrier::activateRelay(relay));
        CHECK((output & RELAYS) == (1U << (relay + 9U)));
        CHECK(Carrier::commandedRelays() == (1U << (relay - 1U)));
        CHECK(calls == std::vector<std::string>({"clear_relays", "high" + std::to_string(relay + 9U)}));
    }
    // Selected OFF must preserve every unrelated output/command bit.
    for (unsigned active = 1; active <= 4; ++active) {
        CHECK(Carrier::activateRelay(active));
        auto const before = output;
        for (unsigned relay = 1; relay <= 4; ++relay) {
            if (relay == active) continue;
            CHECK(Carrier::deactivateRelay(relay));
            CHECK(output == before && Carrier::commandedRelays() == (1U << (active - 1U)));
        }
        for (unsigned invalid : {0U, 5U, 100U}) {
            calls.clear(); CHECK(!Carrier::deactivateRelay(invalid));
            CHECK(calls.empty() && output == before);
            CHECK(Carrier::commandedRelays() == (1U << (active - 1U)));
        }
        calls.clear(); CHECK(Carrier::deactivateRelay(active));
        CHECK(output == ~RELAYS && Carrier::commandedRelays() == 0);
        CHECK(calls == std::vector<std::string>{"clear_selected"});
        CHECK(Carrier::deactivateRelay(active)); // Already OFF is harmless.
    }
    for (unsigned invalid : {0U, 5U, 100U}) {
        calls.clear();
        CHECK(!Carrier::activateRelay(invalid));
        CHECK(!(output & RELAYS) && Carrier::commandedRelays() == 0);
        CHECK(calls == std::vector<std::string>{"clear_relays"});
    }
    CHECK(Carrier::activateRelay(4));
    CHECK(!BSP::Detail::invokeSerialLossAction());
    BSP::setSerialLossAction(&Carrier::forceRelaysOff);
    CHECK(BSP::Detail::invokeSerialLossAction());
    CHECK(!(output & RELAYS) && Carrier::commandedRelays() == 0);
    BSP::setSerialLossAction(nullptr);
    CHECK(!BSP::Detail::invokeSerialLossAction());

    QP::QF::init(); QP::QF::onStartup(); // Real hook, mocked successful timer.
    BSP::setFaultAction(&trap);
    for (bool initializationError : {false, true}) {
        CHECK(Carrier::activateRelay(1)); CHECK(Carrier::commandedRelays() == 1);
        output = ~0U; irqDisabled = false; calls.clear();
        try {
            if (initializationError) { timerFail = true; QP::QF::onStartup(); }
            else Q_onError("host_framework_fault", 1);
            CHECK(false);
        } catch (Halt const &) {}
        CHECK(irqDisabled && output == ~RELAYS && Carrier::commandedRelays() == 0);
        CHECK(calls == std::vector<std::string>{"clear_relays"});
    }
    std::cout << "Carrier routing/order/pulls and actual framework/init fault hooks passed\n";
}
