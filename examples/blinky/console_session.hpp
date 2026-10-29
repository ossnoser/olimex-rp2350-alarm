// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "command_parser.hpp"
#include <cstddef>
#include <cstdint>

namespace Console {
inline constexpr char PROMPT[] = "Enter duty cycle (1..9, 10%..90% on):\r\n";

struct Connection {
    bool connected;
    std::uint32_t generation;
};

// Small test seam; the firmware adapter delegates exclusively to BSP.
class Port {
public:
    virtual void service() = 0;
    virtual Connection connection() = 0;
    virtual bool read(char &byte) = 0;
    virtual std::size_t write(char const *bytes, std::size_t count) = 0;
protected:
    ~Port() = default;
};

class Session {
public:
    using Accept = void (*)(Commands::DutyCycle);
    void poll(Port &port, Accept accept);

private:
    static constexpr std::size_t TX_SIZE = 256;
    static constexpr std::size_t RESPONSE_RESERVE = 80;
    static_assert(RESPONSE_RESERVE >= sizeof(PROMPT) - 1U + 12U);
    static_assert(TX_SIZE >= RESPONSE_RESERVE);
    char line_[32]{};
    std::size_t length_ = 0;
    bool invalid_ = false;
    bool skipLf_ = false;
    bool connected_ = false;
    std::uint32_t generation_ = 0;
    char tx_[TX_SIZE]{};
    std::size_t head_ = 0;
    std::size_t count_ = 0;

    void reset();
    void append(char const *text);
    void append(char byte);
    void drain(Port &port, std::size_t &budget);
    bool consume(char byte, Accept accept);
};
} // namespace Console
