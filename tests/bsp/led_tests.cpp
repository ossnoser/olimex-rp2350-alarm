// SPDX-License-Identifier: GPL-3.0-or-later
#include "bsp.hpp"
#include "board.hpp"
#include "qsafe.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#define CHECK(condition) do { if (!(condition)) { \
    std::cerr << __LINE__ << ": " #condition "\n"; std::exit(1); } } while (false)

namespace {
struct Call { std::string kind; unsigned pin; bool value; };
struct Fault {};
std::vector<Call> calls;
int cywResult = 0;
std::string faultSource;
int faultId = 0;
void expect(unsigned index, char const *kind, unsigned pin, bool value) {
    CHECK(index < calls.size());
    CHECK(calls[index].kind == kind && calls[index].pin == pin && calls[index].value == value);
}
}
void gpio_init(unsigned pin) { calls.push_back({"gpio_init", pin, false}); }
void gpio_put(unsigned pin, bool value) { calls.push_back({"gpio_put", pin, value}); }
void gpio_set_dir(unsigned pin, bool output) { calls.push_back({"gpio_dir", pin, output}); }
int cyw43_arch_init() { calls.push_back({"cyw_init", 0, false}); return cywResult; }
void cyw43_arch_gpio_put(unsigned pin, bool value) { calls.push_back({"cyw_put", pin, value}); }
extern "C" [[noreturn]] void Q_onError(char const * const module, int_t const id) {
    faultSource = module; faultId = id; throw Fault{};
}

int main() {
    BSP::Board::initLed();
#if EXPECT_WIRELESS
    CHECK(calls.size() == 1);
    expect(0, "cyw_init", 0, false);
    CHECK(std::string(BSP::Board::usbProductName()) == "Pico 2 W Blinky");
    calls.clear(); BSP::setLed(false); BSP::setLed(true);
    CHECK(calls.size() == 2);
    expect(0, "cyw_put", CYW43_WL_GPIO_LED_PIN, false);
    expect(1, "cyw_put", CYW43_WL_GPIO_LED_PIN, true);
    calls.clear(); cywResult = -7;
    try { BSP::Board::initLed(); CHECK(false); } catch (Fault const &) {}
    CHECK(faultSource == "cyw43_init" && faultId == 1);
    CHECK(calls.size() == 1);
#else
    CHECK(calls.size() == 3);
    expect(0, "gpio_init", PICO_DEFAULT_LED_PIN, false);
    expect(1, "gpio_put", PICO_DEFAULT_LED_PIN, false);
    expect(2, "gpio_dir", PICO_DEFAULT_LED_PIN, true);
    CHECK(std::string(BSP::Board::usbProductName()) == "Pico 2 Blinky");
    calls.clear(); BSP::setLed(false); BSP::setLed(true);
    CHECK(calls.size() == 2);
    expect(0, "gpio_put", PICO_DEFAULT_LED_PIN, false);
    expect(1, "gpio_put", PICO_DEFAULT_LED_PIN, true);
#endif
    std::cout << "Selected LED adapter, initialization and USB product identity passed\n";
}
