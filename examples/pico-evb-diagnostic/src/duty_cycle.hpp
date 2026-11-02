// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Independent data type for the copied HSM's retained, unused DUTY handler.
// There is deliberately no command parser in this application.
namespace Commands {
struct DutyCycle { unsigned tenths; };
}
