// SPDX-License-Identifier: GPL-3.0-or-later
#include "bsp.hpp"
#include "board.hpp"
#include "runtime_options.hpp"
#include "qsafe.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#define CHECK(c) do { if (!(c)) { std::cerr << __LINE__ << ": " #c "\n"; std::exit(1); } } while(false)
extern "C" void tud_mount_cb();
extern "C" void tud_umount_cb();
extern "C" void tud_suspend_cb(bool);
extern "C" void tud_cdc_line_state_cb(std::uint8_t, bool, bool);
namespace { std::vector<std::string> calls; bool connected = true; void off() { calls.push_back("OFF"); } }
bool tusb_init() { return true; }
void tud_task_ext(std::uint32_t timeout, bool irq) { CHECK(timeout == 0 && !irq); }
bool tud_cdc_connected() { return connected; }
void tud_cdc_read_flush() { calls.push_back("RX"); }
void tud_cdc_write_clear() { calls.push_back("TX"); }
std::uint32_t tud_cdc_write_flush() { return 0; }
std::uint32_t tud_cdc_write_available() { return 64; }
std::uint32_t tud_cdc_read(void *, std::uint32_t) { return 0; }
std::uint32_t tud_cdc_write(void const *, std::uint32_t n) { return n; }
void BSP::Board::initLed() {}
char const *BSP::Board::usbProductName() { return "Default"; }
void BSP::setLed(bool on) { CHECK(!on); }
extern "C" [[noreturn]] void Q_onError(char const *const, int_t const) { std::abort(); }
int main() {
    BSP::init();
    tud_mount_cb(); auto generation = BSP::serialGeneration();
    CHECK(calls == std::vector<std::string>({"RX","TX"}));
    calls.clear(); tud_suspend_cb(false);
    CHECK(calls.empty() && BSP::serialGeneration() == generation); // Default unchanged.
    BSP::setSerialLossAction(&off);
    for (unsigned event=0; event<4; ++event) {
        calls.clear(); generation = BSP::serialGeneration();
        if (event == 0) tud_cdc_line_state_cb(0, true, false);
        if (event == 1) tud_suspend_cb(false);
        if (event == 2) tud_umount_cb();
        if (event == 3) tud_mount_cb();
        CHECK(calls == std::vector<std::string>({"OFF","RX","TX"}));
        CHECK(BSP::serialGeneration() == generation + 1);
    }
    BSP::setSerialLossAction(nullptr); calls.clear(); generation = BSP::serialGeneration();
    tud_suspend_cb(false); CHECK(calls.empty() && BSP::serialGeneration() == generation);
    std::cout << "Real transport callbacks: opted-in OFF/session invalidation and unchanged defaults passed\n";
}
