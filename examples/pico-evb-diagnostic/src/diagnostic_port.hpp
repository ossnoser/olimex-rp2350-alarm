// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "report_session.hpp"
#include "bsp.hpp"
#include "blinky.hpp"
#include "diagnostic_support.hpp"
class DiagnosticPort final : public Report::Port {
public:
    void service() override { DiagnosticSupport::serviceTransport(); }
    Report::Connection connection() override {
        return {BSP::serialConnected(), BSP::serialGeneration()};
    }
    bool read(char &byte) override { return BSP::serialRead(byte); }
    std::size_t write(char const *data, std::size_t count) override {
        return BSP::serialWrite(data, count);
    }
    Carrier::Sample sample() override { return Carrier::sample(); }
    unsigned revision() override { return Carrier::siliconRevision(); }
    RelayTest::State relayState() override { return Blinky::relayState(); }
    unsigned activeRelay() override { return Blinky::activeRelay(); }
    std::uint8_t commandedRelays() override { return Carrier::commandedRelays(); }
    bool requestOn(unsigned relay) override { return Blinky::requestOn(relay); }
    void stopRelay(unsigned relay) override { Blinky::stopRelay(relay); }
    void stopAll() override { Blinky::stopAll(); }
};
