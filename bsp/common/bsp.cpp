// SPDX-License-Identifier: GPL-3.0-or-later
#include "bsp.hpp"
#include "board.hpp"
#include "runtime_options.hpp"
#include "qsafe.h"
#include "tusb.h"
#include <algorithm>
#include <atomic>

namespace {
using Wakeup = void (*)();
static_assert(std::atomic<Wakeup>::is_always_lock_free);
std::atomic<Wakeup> serialWakeup{nullptr};
std::uint32_t sessionGeneration = 0;
bool terminalDtr = false;

void resetSession() {
    static_cast<void>(BSP::Detail::invokeSerialLossAction());
    ++sessionGeneration;
    tud_cdc_read_flush();
    tud_cdc_write_clear();
}
}

void BSP::init() {
    Board::initLed();
    setLed(false);
    if (!tusb_init()) {
        Q_onError("usb_init", 1);
    }
}

void BSP::setSerialWakeup(void (*notify)()) {
    serialWakeup.store(notify, std::memory_order_release);
}

void BSP::serialService() {
    // Poll and USB-service events run in the receiving AO's application context.
    tud_task_ext(0, false);
    if (serialConnected()) {
        tud_cdc_write_flush(); // Submit a transfer; does not wait for the host.
    }
}

bool BSP::serialConnected() {
    return tud_cdc_connected();
}

std::uint32_t BSP::serialGeneration() {
    return sessionGeneration;
}

bool BSP::serialRead(char &byte) {
    return serialConnected() && tud_cdc_read(&byte, 1) == 1;
}

std::size_t BSP::serialWrite(char const *bytes, std::size_t count) {
    if (!serialConnected()) {
        return 0;
    }
    auto const available = tud_cdc_write_available();
    auto const offered = static_cast<std::uint32_t>(std::min<std::size_t>(count, available));
    if (offered == 0) {
        return 0;
    }
    std::uint32_t const accepted = tud_cdc_write(bytes, offered);
    tud_cdc_write_flush();
    return accepted;
}

// TinyUSB calls this after enqueuing work, possibly from the USB IRQ.
// The registered callback only posts a coalesced QP event.
extern "C" void tud_event_hook_cb(std::uint8_t, std::uint32_t, bool) {
    if (auto notify = serialWakeup.load(std::memory_order_acquire)) {
        notify();
    }
}

extern "C" void tud_cdc_line_state_cb(std::uint8_t, bool dtr, bool) {
    if (terminalDtr != dtr) {
        terminalDtr = dtr;
        resetSession();
    }
}

extern "C" void tud_mount_cb() {
    terminalDtr = false;
    resetSession();
}

extern "C" void tud_suspend_cb(bool) {
    // Default users retain their old session behavior. The relay-test opt-in
    // clears outputs and invalidates even a suspend/resume between two polls.
    if (BSP::Detail::invokeSerialLossAction()) {
        ++sessionGeneration;
        tud_cdc_read_flush();
        tud_cdc_write_clear();
    }
}

extern "C" void tud_umount_cb() {
    terminalDtr = false;
    resetSession();
}
