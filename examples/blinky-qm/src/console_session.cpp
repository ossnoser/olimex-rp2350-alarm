// SPDX-License-Identifier: GPL-3.0-or-later
#include "console_session.hpp"
#include <algorithm>

void Console::Session::reset() {
    length_ = 0;
    invalid_ = false;
    skipLf_ = false;
    head_ = 0;
    count_ = 0;
}

void Console::Session::append(char byte) {
    tx_[(head_ + count_) % TX_SIZE] = byte;
    ++count_;
}

void Console::Session::append(char const *text) {
    while (*text != '\0') {
        append(*text++);
    }
}

void Console::Session::drain(Port &port, std::size_t &budget) {
    std::size_t const offered = std::min({count_, TX_SIZE - head_, budget});
    if (offered != 0) {
        std::size_t const written = port.write(tx_ + head_, offered);
        head_ = (head_ + written) % TX_SIZE;
        count_ -= written;
        // Bound bytes offered, not merely bytes successfully sent.
        budget -= offered;
    }
}

bool Console::Session::consume(char byte, Accept accept) {
    if (skipLf_) {
        skipLf_ = false;
        if (byte == '\n') {
            return false;
        }
    }
    if (byte == '\r' || byte == '\n') {
        skipLf_ = byte == '\r';
        append("\r\n");
        Commands::DutyCycle duty{};
        if (!invalid_ && Commands::parse(line_, length_, duty)) {
            accept(duty);
            append("Accepted\r\n");
        } else {
            append("Invalid\r\n");
        }
        append(PROMPT);
        length_ = 0;
        invalid_ = false;
        return true;
    }
    if (invalid_) {
        return false; // Only Enter recovers an invalid/overflowed line.
    }
    if (byte == '\b' || byte == '\x7f') {
        if (length_ != 0) {
            --length_;
            append("\b \b");
        }
    } else if (static_cast<unsigned char>(byte) < 0x20U
               || static_cast<unsigned char>(byte) > 0x7eU) {
        invalid_ = true;
    } else if (length_ == sizeof line_) {
        invalid_ = true;
    } else {
        line_[length_++] = byte;
        append(byte);
    }
    return false;
}

void Console::Session::poll(Port &port, Accept accept) {
    port.service();
    Connection const state = port.connection();
    if (!state.connected) {
        reset();
        connected_ = false;
        generation_ = state.generation;
        return;
    }
    if (!connected_ || state.generation != generation_) {
        reset();
        connected_ = true;
        generation_ = state.generation;
        append(PROMPT);
    }
    std::size_t budget = 64;
    drain(port, budget);
    for (unsigned i = 0; i < 16 && TX_SIZE - count_ >= RESPONSE_RESERVE; ++i) {
        char byte;
        if (!port.read(byte)) {
            break;
        }
        if (consume(byte, accept)) {
            break; // Yield to higher-priority Blinky after one submission.
        }
    }
    drain(port, budget);
}
