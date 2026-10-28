// SPDX-License-Identifier: GPL-3.0-or-later
// This file is licensed under the GNU General Public License, version 3
// or (at your option) any later version.
#pragma once

#define QF_MAX_ACTIVE 1U
#define QF_MAX_TICK_RATE 1U

// The GNU Arm QV port uses PRIMASK when QF_USE_BASEPRI is undefined.
#ifdef QF_USE_BASEPRI
#error "This integration requires the approved PRIMASK interrupt policy."
#endif

// Q_UNSAFE disables QP safety assertions independently of the build type.
#ifdef Q_UNSAFE
#error "QP assertions must remain enabled in all build configurations."
#endif

#ifdef Q_SPY
#error "QS tracing is disabled for this initial integration."
#endif
