// SPDX-License-Identifier: GPL-3.0-or-later
#include "carrier.hpp"
#include "hardware/gpio.h"
#include "pico/platform.h"
#include "pico/time.h"
#include <atomic>

// A nonstandard LED override could blink a relay/input pin. Reject it for
// this carrier profile rather than silently violate the relay-off contract.
static_assert(PICO_DEFAULT_LED_PIN == 25U,
              "PICO-EVB diagnostic requires the stock Pico 2 GP25 onboard LED");

namespace {
static_assert(std::atomic<std::uint32_t>::is_always_lock_free);
std::atomic<std::uint32_t> relayCommands{0U};
constexpr unsigned RELAYS[] = {10U, 11U, 12U, 13U};
constexpr unsigned INPUTS[] = {2U, 3U, 8U, 9U};
constexpr std::uint32_t RELAY_MASK = (1U << 10U) | (1U << 11U)
                                  | (1U << 12U) | (1U << 13U);
}
void Carrier::initRelaysOff() {
    // Clear all SIO latches together before touching any pin configuration.
    forceRelaysOff();
    for (auto pin : RELAYS) {
        // gpio_init selects SIO with output disabled and latch cleared.
        gpio_init(pin);
        gpio_put(pin, false);
        gpio_set_dir(pin, GPIO_OUT);
    }
}
void Carrier::forceRelaysOff() {
    // One bounded SIO latch clear. Startup selects SIO/output afterwards;
    // faults call after that configuration. Never reconfigure pins, allocate,
    // call QP, wait or touch USB here.
    gpio_clr_mask(RELAY_MASK);
    relayCommands.store(0U, std::memory_order_release);
}
bool Carrier::activateRelay(unsigned relay) {
    forceRelaysOff(); // Never overlap commanded coils, even on invalid requests.
    if (relay < 1U || relay > 4U) return false;
    gpio_put(RELAYS[relay - 1U], true);
    relayCommands.store(1U << (relay - 1U), std::memory_order_release);
    return true;
}
bool Carrier::deactivateRelay(unsigned relay) {
    if (relay < 1U || relay > 4U) return false;
    gpio_clr_mask(1U << RELAYS[relay - 1U]);
    relayCommands.fetch_and(~(1U << (relay - 1U)), std::memory_order_acq_rel);
    return true;
}
std::uint8_t Carrier::commandedRelays() {
    return static_cast<std::uint8_t>(relayCommands.load(std::memory_order_acquire));
}
void Carrier::initInputs() {
    for (auto pin : INPUTS) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_disable_pulls(pin);
    }
}
Carrier::Sample Carrier::sample() {
    auto const milliseconds = time_us_64() / 1000U;
    auto const levels = gpio_get_all(); // One register snapshot for all inputs.
    std::uint8_t mask = 0;
    for (unsigned i = 0; i < 4U; ++i) {
        mask |= static_cast<std::uint8_t>(((levels >> INPUTS[i]) & 1U) << i);
    }
    return {milliseconds, mask};
}
unsigned Carrier::siliconRevision() {
    return rp2350_chip_version();
}
