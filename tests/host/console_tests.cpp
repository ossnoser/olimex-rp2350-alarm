// SPDX-License-Identifier: GPL-3.0-or-later
#include "command_parser.hpp"
#include "console_session.hpp"
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <string>
#include <vector>

#define CHECK(condition) do { if (!(condition)) { \
    std::cerr << __LINE__ << ": " #condition "\n"; std::exit(1); } } while (false)

namespace {
std::vector<Commands::DutyCycle> accepted;
void accept(Commands::DutyCycle duty) { accepted.push_back(duty); }

struct FakePort : Console::Port {
    Console::Connection status{true, 1};
    std::deque<char> rx;
    std::string tx;
    std::size_t writeLimit = 64;
    unsigned readCount = 0;
    std::size_t offered = 0;
    void service() override { readCount = 0; offered = 0; }
    Console::Connection connection() override { return status; }
    bool read(char &byte) override {
        if (rx.empty()) return false;
        byte = rx.front(); rx.pop_front(); ++readCount; return true;
    }
    std::size_t write(char const *bytes, std::size_t count) override {
        offered += count;
        auto const size = std::min(count, writeLimit);
        tx.append(bytes, size);
        return size;
    }
    void feed(std::string const &text) { rx.insert(rx.end(), text.begin(), text.end()); }
};

struct Harness {
    FakePort port;
    Console::Session session;
    Harness() { accepted.clear(); poll(); port.tx.clear(); }
    void poll(unsigned count = 1) {
        while (count-- != 0) {
            auto before = accepted.size();
            session.poll(port, accept);
            CHECK(port.readCount <= 16);
            CHECK(port.offered <= 64);
            CHECK(accepted.size() - before <= 1);
        }
    }
    void send(std::string const &text) { port.feed(text); poll(100); }
};

std::size_t occurrences(std::string const &text, std::string const &needle) {
    std::size_t count = 0, pos = 0;
    while ((pos = text.find(needle, pos)) != std::string::npos) {
        ++count; pos += needle.size();
    }
    return count;
}

void parserTests() {
    for (unsigned duty = 1; duty <= 9; ++duty) {
        for (std::string text : {std::to_string(duty), "  " + std::to_string(duty) + "  "}) {
            Commands::DutyCycle result{};
            CHECK(Commands::parse(text.data(), text.size(), result));
            CHECK(result.tenths == duty);
        }
    }
    for (std::string text : {"", "   ", "0", "10", "99", "01", "05", "1,5", "7,8",
            "+1", "-1", "1.5", "1,", ",2", "12", "1x", "1\t", "1\n", "1\r",
            "a", "1;2", "1 2"}) {
        Commands::DutyCycle result{42};
        CHECK(!Commands::parse(text.data(), text.size(), result));
        CHECK(result.tenths == 42);
    }
    char const embeddedNull[] = {'1', '\0', ' '};
    Commands::DutyCycle result{};
    CHECK(!Commands::parse(embeddedNull, sizeof embeddedNull, result));
}

void lineTests() {
    for (std::string ending : {"\r", "\n", "\r\n"}) {
        Harness h;
        h.send(" 7 " + ending);
        CHECK(accepted.size() == 1 && accepted[0].tenths == 7);
        CHECK(h.port.tx == " 7 \r\nAccepted\r\n" + std::string(Console::PROMPT));
    }
    Harness h;
    h.send("\r\n\n \r\n");
    CHECK(accepted.empty());
    CHECK(occurrences(h.port.tx, "Invalid\r\n") == 3);
    CHECK(occurrences(h.port.tx, Console::PROMPT) == 3);
    h.port.tx.clear();
    h.send("1\r"); h.send("\n");
    CHECK(accepted.size() == 1);
    CHECK(occurrences(h.port.tx, "Accepted\r\n") == 1);
    h.port.tx.clear();
    h.port.feed("1\n2\n3\n");
    h.poll();
    CHECK(accepted.size() == 2 && !h.port.rx.empty());
    h.poll(20);
    CHECK(accepted.size() == 4 && accepted.back().tenths == 3);
    for (unsigned duty = 1; duty <= 9; ++duty) h.send(std::to_string(duty) + "\n");
    CHECK(accepted.size() == 13);
    for (unsigned duty = 1; duty <= 9; ++duty) CHECK(accepted[3 + duty].tenths == duty);
    h.send("0\n1,5\n10\n");
    CHECK(accepted.size() == 13);
}

void editingAndBoundsTests() {
    Harness h;
    h.send("\b\x7f" "4\b5\n");
    CHECK(accepted.size() == 1 && accepted.back().tenths == 5);
    CHECK(h.port.tx.find("4\b \b5") != std::string::npos);
    h.send("  \x7f\b3\n");
    CHECK(accepted.size() == 2 && accepted.back().tenths == 3);
    h.send(std::string(31, ' ') + "5\n"); // Exactly 32 entered characters.
    CHECK(accepted.size() == 3);
    for (std::string text : std::vector<std::string>{std::string(32, ' ') + "5\b\n",
                            "1\t\b\n", "2\x1b[A\n"}) {
        auto before = accepted.size();
        h.port.tx.clear(); h.send(text);
        CHECK(accepted.size() == before);
        CHECK(occurrences(h.port.tx, "Invalid\r\n") == 1);
        h.send("4\n");
        CHECK(accepted.size() == before + 1);
    }
}

void bufferingTests() {
    Harness h;
    h.port.writeLimit = 0;
    for (unsigned i = 0; i < 200; ++i) h.port.feed("9\n");
    h.poll(300);
    CHECK(!h.port.rx.empty());
    CHECK(accepted.size() < 10);
    CHECK(h.port.tx.empty());
    h.port.writeLimit = 1; // Partial writes and ring-buffer wrapping.
    h.poll(20000);
    CHECK(h.port.rx.empty() && accepted.size() == 200);
    std::string expected;
    for (unsigned i = 0; i < 200; ++i)
        expected += "9\r\nAccepted\r\n" + std::string(Console::PROMPT);
    CHECK(h.port.tx == expected);
}

void connectionTests() {
    Harness h;
    h.port.writeLimit = 0;
    h.send("3\n4");
    CHECK(accepted.size() == 1);
    h.port.status = {false, 2}; h.port.rx.clear(); h.poll();
    h.port.status = {true, 3}; h.port.writeLimit = 64; h.poll(10);
    CHECK(h.port.tx == Console::PROMPT);
    h.send("1\n");
    CHECK(accepted.size() == 2 && accepted.back().tenths == 1); // Not old "41".
    h.port.tx.clear();
    h.send("5"); h.port.tx.clear();
    ++h.port.status.generation;
    h.poll(10);
    CHECK(h.port.tx == Console::PROMPT);
    h.send("2\n");
    CHECK(accepted.size() == 3 && accepted.back().tenths == 2);
}
}

int main() {
    parserTests(); lineTests(); editingAndBoundsTests(); bufferingTests(); connectionTests();
    std::cout << "Duty-cycle parser and console tests passed\n";
}
