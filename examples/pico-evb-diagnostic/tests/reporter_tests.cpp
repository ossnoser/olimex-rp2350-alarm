// SPDX-License-Identifier: GPL-3.0-or-later
#include "report_session.hpp"
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <limits>
#include <string>

#define CHECK(c) do { if (!(c)) { std::cerr << __LINE__ << ": " #c "\n"; std::exit(1); } } while (false)
struct Fake : Report::Port {
    Report::Connection link{true, 1};
    Carrier::Sample value{20, 15};
    unsigned silicon = 2, samples = 0, services = 0, reads = 0;
    unsigned requests = 0, stops = 0;
    RelayTest::State state = RelayTest::State::Ready;
    std::uint8_t mask = 0;
    unsigned step = 1;
    std::size_t credit = 1000, written = 0;
    bool invalidWrite = false;
    std::deque<char> rx;
    std::string tx;
    void service() override { ++services; }
    Report::Connection connection() override { return link; }
    bool read(char &byte) override {
        ++reads;
        if (rx.empty()) return false;
        byte = rx.front(); rx.pop_front(); return true;
    }
    std::size_t write(char const *data, std::size_t count) override {
        if (invalidWrite) return count + 1;
        auto const n = std::min(count, credit);
        tx.append(data, n); credit -= n; written += n; return n;
    }
    Carrier::Sample sample() override { ++samples; return value; }
    unsigned revision() override { return silicon; }
    RelayTest::State relayState() override { return state; }
    unsigned sequenceStep() override { return step; }
    std::uint8_t commandedRelays() override { return mask; }
    bool requestSequence() override {
        ++requests;
        if (state != RelayTest::State::Ready) return false;
        state = RelayTest::State::Pulsing; mask = 1; return true;
    }
    void stopTest() override { ++stops; state = RelayTest::State::Locked; mask = 0; }
    void poll(Report::Session &session, std::size_t capacity = 1000) {
        credit = capacity; written = reads = 0;
        auto const before = samples;
        CHECK(session.poll(*this));
        CHECK(samples == before + 1 && written <= 64 && reads <= 16);
        value.milliseconds += 20;
    }
    void flush(Report::Session &session) { for (unsigned i = 0; i < 20; ++i) poll(session); }
};
int main() {
    Report::Session session;
    Fake port;
    port.rx.insert(port.rx.end(), 100, '9');
    port.flush(session);
    CHECK(port.rx.empty());
    CHECK(port.tx.find("Pico 2 EVB Diagnostic") != std::string::npos);
    CHECK(port.tx.find("raw revision=2 (A2)") != std::string::npos);
    CHECK(port.tx.find("WARNING: GPIO E9 applies") != std::string::npos);
    CHECK(port.tx.find("no contact feedback") != std::string::npos);
    CHECK(port.tx.find("t=20ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH\r\n") != std::string::npos);
    auto stable = port.tx;
    port.flush(session); CHECK(port.tx == stable);

    // All sixteen bit patterns, exact labels, and no echo/command interpretation.
    for (unsigned mask = 0; mask < 16; ++mask) {
        port.tx.clear(); port.value.highMask = mask;
        auto const timestamp = port.value.milliseconds;
        port.poll(session);
        std::string expected = "t=" + std::to_string(timestamp) + "ms";
        for (unsigned i = 0; i < 4; ++i)
            expected += " IN" + std::to_string(i + 1) + "=" + ((mask & (1U << i)) ? "HIGH" : "LOW");
        CHECK(port.tx == expected + "\r\n");
    }
    // Pending unsent snapshot replaced with the newest, while sampling continues.
    port.tx.clear(); port.value.highMask = 0; port.poll(session, 0);
    port.value.highMask = 1; port.poll(session, 0);
    port.value.highMask = 2; auto newest = port.value.milliseconds; port.poll(session, 0);
    port.flush(session);
    CHECK(port.tx == "t=" + std::to_string(newest) + "ms IN1=LOW IN2=HIGH IN3=LOW IN4=LOW\r\n");

    // A partially sent line must finish before one coalesced latest line.
    port.tx.clear(); port.value.highMask = 3;
    auto const first = port.value.milliseconds; port.poll(session, 7);
    CHECK(port.tx.size() == 7);
    port.value.highMask = 4; port.poll(session, 0);
    port.value.highMask = 8; newest = port.value.milliseconds; port.poll(session, 0);
    port.flush(session);
    CHECK(port.tx == "t=" + std::to_string(first) + "ms IN1=HIGH IN2=HIGH IN3=LOW IN4=LOW\r\n"
        + "t=" + std::to_string(newest) + "ms IN1=LOW IN2=LOW IN3=LOW IN4=HIGH\r\n");

    // Reopen between polls (same connected boolean, changed generation) resets.
    for (unsigned revision : {3U, 42U}) {
        port.tx.clear(); ++port.link.generation; port.silicon = revision;
        port.flush(session);
        CHECK(port.tx.rfind("\r\nPico 2 EVB Diagnostic", 0) == 0);
        CHECK(port.tx.find(revision == 3 ? "(A3/A4)" : "(unknown)") != std::string::npos);
        CHECK(port.tx.find("WARNING: GPIO E9 applies") == std::string::npos);
        CHECK(port.tx.find(revision == 3 ? "fixed in A3/A4" : "unknown revision") != std::string::npos);
    }
    port.value.highMask = 0; port.poll(session, 3); // Interrupted snapshot.
    port.link.connected = false; port.tx.clear(); port.poll(session);
    port.value.highMask = 15; port.poll(session); CHECK(port.tx.empty());
    port.link.connected = true; ++port.link.generation;
    port.value.milliseconds = std::numeric_limits<std::uint64_t>::max();
    CHECK(session.poll(port)); // Ensure maximum timestamp fits fixed buffer.
    port.flush(session);
    CHECK(port.tx.find("t=18446744073709551615ms") != std::string::npos);
    CHECK(port.tx.find("IN1=LOW") == std::string::npos);
    for (std::size_t i = 0; i < port.tx.size(); ++i) {
        if (port.tx[i] == '\n') CHECK(i > 0 && port.tx[i - 1] == '\r');
    }
    // Control is independent of blocked output, exact commands and CRLF framing.
    Report::Session commands;
    Fake ctl;
    auto feed = [&](std::string const &s) { ctl.rx.insert(ctl.rx.end(), s.begin(), s.end()); };
    ctl.flush(commands); ctl.tx.clear();
    feed("test1\ntest2\nTESTALL\n9\n");
    for (unsigned i=0;i<5;++i) ctl.poll(commands, 0);
    CHECK(ctl.requests == 0 && ctl.stops == 0);
    feed("testall\r\noff\r\n");
    ctl.poll(commands, 0); CHECK(ctl.mask == 1 && ctl.requests == 1);
    ctl.poll(commands, 0); CHECK(ctl.mask == 0 && ctl.stops == 1);
    ctl.poll(commands, 0); CHECK(ctl.stops == 1); // LF is not another command.
    feed("testall\n"); ctl.poll(commands, 0);
    CHECK(ctl.requests == 2 && ctl.mask == 0); // Fake rejects LOCKED.
    ctl.flush(commands);
    CHECK(ctl.tx.find("LOCKED") != std::string::npos);
    feed(std::string(100, 'x') + "\n");
    for(unsigned i=0;i<8;++i) ctl.poll(commands);
    CHECK(ctl.requests == 2 && ctl.tx.find("Invalid") != std::string::npos);
    // A partial command cannot cross a connection generation boundary.
    Report::Session fresh;
    Fake reconnect;
    reconnect.rx = {'t','e','s','t'}; reconnect.poll(fresh);
    ++reconnect.link.generation; reconnect.rx = {'a','l','l','\n'};
    reconnect.poll(fresh); CHECK(reconnect.requests == 0);
    // Four-bit command/status formatting remains explicit, including OFF gaps.
    Report::Session status;
    Fake outputs; outputs.flush(status);
    for (unsigned relay = 1; relay <= 4; ++relay) {
        outputs.tx.clear(); outputs.step = relay;
        outputs.state = RelayTest::State::Pulsing;
        outputs.mask = static_cast<std::uint8_t>(1U << (relay - 1U));
        outputs.flush(status);
        std::string bits = "CMD=";
        for (unsigned i = 1; i <= 4; ++i) bits += std::string(i == relay ? "ON" : "OFF") + (i < 4 ? "," : "");
        CHECK(outputs.tx.find(bits) != std::string::npos);
        CHECK(outputs.tx.find("STEP=" + std::to_string(relay)) != std::string::npos);
        outputs.tx.clear(); outputs.mask = 0;
        outputs.state = relay < 4 ? RelayTest::State::Gap : RelayTest::State::Locked;
        outputs.flush(status);
        CHECK(outputs.tx.find("CMD=OFF,OFF,OFF,OFF") != std::string::npos);
        CHECK(outputs.tx.find(relay < 4 ? "SEQ=GAP" : "SEQ=LOCKED") != std::string::npos);
    }
    Report::Session broken;
    Fake bad; bad.invalidWrite = true;
    CHECK(!broken.poll(bad));
    std::cout << "Reporter bounds, raw values, coalescing, partial lines and reconnect passed\n";
}
