// SPDX-License-Identifier: GPL-3.0-or-later
#include "blinky.hpp"
#include "bsp.hpp"
#include "qp_port.hpp"

int main() {
    QP::QF::init();
    BSP::init();
    Blinky::start();
    return QP::QF::run();
}
