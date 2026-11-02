// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
namespace BSP::Detail {
// Internal hooks; invokeFaultAction is also exercised without the halt loop.
void invokeFaultAction();
// True only when an opted-in action was invoked.
bool invokeSerialLossAction();
char const *usbProductName();
}
