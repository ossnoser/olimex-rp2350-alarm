// SPDX-License-Identifier: GPL-3.0-or-later
#include "bsp.hpp"
#include "board.hpp"
#include "runtime_options.hpp"
#include <atomic>

namespace {
using Action = void (*)();
static_assert(std::atomic<Action>::is_always_lock_free);
std::atomic<Action> faultAction{nullptr};
std::atomic<Action> serialLossAction{nullptr};
// Written once before USB initialization, then read only in application context.
char const *productName = nullptr;
}

void BSP::setFaultAction(void (*action)()) {
    faultAction.store(action, std::memory_order_release);
}
void BSP::setSerialLossAction(void (*action)()) {
    serialLossAction.store(action, std::memory_order_release);
}
bool BSP::Detail::invokeSerialLossAction() {
    auto const action = serialLossAction.load(std::memory_order_acquire);
    if (action == nullptr) return false;
    action();
    return true;
}
void BSP::Detail::invokeFaultAction() {
    auto const action = faultAction.load(std::memory_order_acquire);
    if (action != nullptr) action();
}
bool BSP::setUsbProductName(char const *name) {
    if (name == nullptr || name[0] == '\0') return false;
    unsigned length = 0;
    while (length < 32U && name[length] != '\0') {
        auto const byte = static_cast<unsigned char>(name[length]);
        if (byte < 32U || byte > 126U) return false;
        ++length;
    }
    if (name[length] != '\0') return false;
    productName = name;
    return true;
}
char const *BSP::Detail::usbProductName() {
    return productName != nullptr ? productName : Board::usbProductName();
}
