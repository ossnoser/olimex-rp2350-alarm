// SPDX-License-Identifier: GPL-3.0-or-later
#include "bsp.hpp"
#include "carrier.hpp"
#include "blinky.hpp"
#include "diagnostic.hpp"
#include "qpcpp.hpp"
#include "qsafe.h"

int main() {
    Carrier::initRelaysOff();
    BSP::setFaultAction(&Carrier::forceRelaysOff);
    BSP::setSerialLossAction(&Carrier::forceRelaysOff);
    Carrier::initInputs();
    QP::QF::init();
    if (!BSP::setUsbProductName("Pico 2 EVB Diagnostic")) {
        Q_onError("diagnostic_identity", 1);
    }
    BSP::init();
    Blinky::start();
    Diagnostic::start();
    return QP::QF::run();
}
