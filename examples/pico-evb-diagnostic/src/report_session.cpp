// SPDX-License-Identifier: GPL-3.0-or-later
#include "report_session.hpp"
#include <algorithm>
#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace {
char const *name(RelayTest::State state) {
    switch (state) {
        case RelayTest::State::Idle: return "IDLE";
        case RelayTest::State::On: return "ON";
    }
    return "UNKNOWN";
}
constexpr char const *onReplies[] = {
    "ON requested: REL1 (manual OFF; no timeout).\r\n",
    "ON requested: REL2 (manual OFF; no timeout).\r\n",
    "ON requested: REL3 (manual OFF; no timeout).\r\n",
    "ON requested: REL4 (manual OFF; no timeout).\r\n"};
constexpr char const *offReplies[] = {
    "OFF requested: REL1 only.\r\n", "OFF requested: REL2 only.\r\n",
    "OFF requested: REL3 only.\r\n", "OFF requested: REL4 only.\r\n"};
}
void Report::Session::resetOutput() {
    length_ = sent_ = commandLength_ = 0;
    invalid_ = skipLf_ = pending_ = controlPending_ = false;
    reply_ = nullptr;
    kind_ = Line::Banner;
}
void Report::Session::receive(Port &port) {
    char byte;
    for (unsigned i = 0; i < 16U && port.read(byte); ++i) {
        if (byte == '\n' && skipLf_) { skipLf_ = false; continue; }
        skipLf_ = false;
        if (byte == '\n' || byte == '\r') {
            skipLf_ = byte == '\r';
            command_[commandLength_] = '\0';
            if (!invalid_ && std::strcmp(command_, "off") == 0) {
                port.stopAll(); // Never conditional on TX space or reply delivery.
                reply_ = "All OFF requested; pending ON cancelled.\r\n";
            } else if (!invalid_ && commandLength_ == 4U
                       && std::memcmp(command_, "on ", 3) == 0
                       && command_[3] >= '1' && command_[3] <= '4') {
                unsigned const relay = static_cast<unsigned>(command_[3] - '0');
                reply_ = port.requestOn(relay) ? onReplies[relay - 1U]
                    : "ON rejected: busy/pending or no active session.\r\n";
            } else if (!invalid_ && commandLength_ == 5U
                       && std::memcmp(command_, "off ", 4) == 0
                       && command_[4] >= '1' && command_[4] <= '4') {
                unsigned const relay = static_cast<unsigned>(command_[4] - '0');
                port.stopRelay(relay);
                reply_ = offReplies[relay - 1U];
            } else {
                reply_ = "Invalid: use on N / off N (N=1..4), or off.\r\n";
            }
            commandLength_ = 0; invalid_ = false;
            break; // At most one command per poll, independent of TX backpressure.
        }
        if (byte < ' ' || byte > '~') invalid_ = true;
        if (commandLength_ < sizeof command_ - 1U) command_[commandLength_++] = byte;
        else invalid_ = true;
    }
}
bool Report::Session::nextLine() {
    sent_ = 0;
    kind_ = Line::Banner;
    int length;
    switch (banner_) {
        case 0:
            length = std::snprintf(tx_, sizeof tx_, "\r\nPico 2 EVB Diagnostic / configured PICO-EVB Rev. A\r\n");
            break;
        case 1:
            length = std::snprintf(tx_, sizeof tx_, "RP2350 raw revision=%u (%s)\r\n", revision_,
                revision_ == 2U ? "A2" : revision_ == 3U ? "A3/A4" : "unknown");
            break;
        case 2:
            length = std::snprintf(tx_, sizeof tx_, "%s\r\n", revision_ == 2U
                ? "WARNING: GPIO E9 applies; validate input voltage margins."
                : revision_ == 3U ? "GPIO E9: fixed in A3/A4."
                : "GPIO E9: unknown revision; consult the datasheet.");
            break;
        case 3:
            length = std::snprintf(tx_, sizeof tx_, "Relay commands: %s %s %s %s (no contact feedback)\r\n",
                (commandMask_ & 1U) ? "ON" : "OFF", (commandMask_ & 2U) ? "ON" : "OFF",
                (commandMask_ & 4U) ? "ON" : "OFF", (commandMask_ & 8U) ? "ON" : "OFF");
            break;
        case 4:
            length = std::snprintf(tx_, sizeof tx_, "Inputs: HIGH=inactive, LOW=active; raw/unfiltered\r\n");
            break;
        case 5:
            length = std::snprintf(tx_, sizeof tx_, "Commands: on N / off N (N=1..4); off (all). No timeout.\r\n");
            break;
        default:
            if (controlPending_) {
                kind_ = Line::Control;
                length = std::snprintf(tx_, sizeof tx_,
                    "t=%" PRIu64 "ms CTRL=%s RELAY=%u CMD=%s,%s,%s,%s (no feedback)\r\n",
                    controlTime_, name(state_), active_,
                    (commandMask_ & 1U) ? "ON" : "OFF", (commandMask_ & 2U) ? "ON" : "OFF",
                    (commandMask_ & 4U) ? "ON" : "OFF", (commandMask_ & 8U) ? "ON" : "OFF");
                controlPending_ = false;
            } else if (pending_) {
                kind_ = Line::Input;
                length = std::snprintf(tx_, sizeof tx_,
                    "t=%" PRIu64 "ms IN1=%s IN2=%s IN3=%s IN4=%s\r\n",
                    latest_.milliseconds,
                    (latest_.highMask & 1U) ? "HIGH" : "LOW",
                    (latest_.highMask & 2U) ? "HIGH" : "LOW",
                    (latest_.highMask & 4U) ? "HIGH" : "LOW",
                    (latest_.highMask & 8U) ? "HIGH" : "LOW");
                pending_ = false;
            } else if (reply_ != nullptr) {
                kind_ = Line::Reply;
                length = std::snprintf(tx_, sizeof tx_, "%s", reply_);
                reply_ = nullptr;
            } else { length_ = 0; return true; }
            break;
    }
    if (banner_ < 6U) ++banner_;
    if (length < 0 || static_cast<std::size_t>(length) >= sizeof tx_) return false;
    length_ = static_cast<std::size_t>(length);
    return true;
}
bool Report::Session::poll(Port &port) {
    port.service();
    auto const sample = port.sample();
    auto const connection = port.connection();
    auto const state = port.relayState();
    auto const commands = port.commandedRelays();
    auto const active = port.activeRelay();
    if (!connection.connected) {
        connected_ = false;
        generation_ = connection.generation;
        resetOutput();
        return true;
    }
    if (!connected_ || generation_ != connection.generation) {
        connected_ = true;
        generation_ = connection.generation;
        resetOutput();
        revision_ = port.revision();
        banner_ = 0;
        pending_ = controlPending_ = true;
        latest_ = sample;
        controlTime_ = sample.milliseconds;
    } else {
        if (previous_ != sample.highMask) { pending_ = true; latest_ = sample; }
        if (state_ != state || commandMask_ != commands || active_ != active) {
            controlPending_ = true; controlTime_ = sample.milliseconds;
        }
    }
    state_ = state; commandMask_ = commands; active_ = active; previous_ = sample.highMask;
    receive(port);
    // Replace only fully unsent lines; complete partial lines intact.
    if (sent_ == 0U && ((kind_ == Line::Input && pending_)
        || (kind_ == Line::Control && controlPending_)
        || (kind_ == Line::Reply && reply_ != nullptr))) length_ = 0;
    std::size_t budget = 64;
    while (budget != 0U) {
        if (sent_ == length_) {
            if (!nextLine()) return false;
            if (length_ == 0U) break;
        }
        auto const requested = std::min(budget, length_ - sent_);
        auto const written = port.write(tx_ + sent_, requested);
        if (written > requested) return false;
        if (written == 0U) break;
        sent_ += written; budget -= written;
    }
    return true;
}
