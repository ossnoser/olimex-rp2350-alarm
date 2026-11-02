// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
namespace DiagnosticSupport {
void serviceUsb();
void serviceTransport(); // Also called by SAMPLE; detect loss without parsing.
}
