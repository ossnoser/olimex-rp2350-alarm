// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "carrier.hpp"
#include "relay_test.hpp"
#include <cstddef>
#include <cstdint>

namespace Report {
struct Connection { bool connected; std::uint32_t generation; };
class Port {
public:
    virtual void service() = 0;
    virtual Connection connection() = 0;
    virtual bool read(char &byte) = 0;
    virtual std::size_t write(char const *data, std::size_t count) = 0;
    virtual Carrier::Sample sample() = 0;
    virtual unsigned revision() = 0;
    virtual RelayTest::State relayState() = 0;
    virtual unsigned sequenceStep() = 0;
    virtual std::uint8_t commandedRelays() = 0;
    virtual bool requestSequence() = 0;
    virtual void stopTest() = 0;
protected:
    ~Port() = default;
};
class Session {
public:
    // Exactly one sample; <=16 RX bytes, one completed command and <=64 TX bytes.
    // False is a detected formatting/port contract error: caller fails OFF.
    bool poll(Port &port);
private:
    enum class Line { Banner, Input, Control, Reply };
    char tx_[256]{};
    std::size_t length_ = 0, sent_ = 0;
    Line kind_ = Line::Banner;
    char command_[16]{};
    std::size_t commandLength_ = 0;
    bool invalid_ = false, skipLf_ = false;
    char const *reply_ = nullptr;
    bool connected_ = false;
    std::uint32_t generation_ = 0;
    unsigned revision_ = 0, banner_ = 0;
    std::uint8_t previous_ = 0;
    Carrier::Sample latest_{};
    bool pending_ = false, controlPending_ = false;
    RelayTest::State state_ = RelayTest::State::Ready;
    std::uint8_t commandMask_ = 0;
    unsigned step_ = 1;
    std::uint64_t controlTime_ = 0;
    bool nextLine();
    void receive(Port &port);
    void resetOutput();
};
}
