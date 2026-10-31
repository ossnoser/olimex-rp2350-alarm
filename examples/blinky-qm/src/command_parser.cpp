// SPDX-License-Identifier: GPL-3.0-or-later
#include "command_parser.hpp"

bool Commands::parse(char const *text, std::size_t length, DutyCycle &result) {
    char digit = '\0';
    for (std::size_t i = 0; i < length; ++i) {
        if (text[i] == ' ') {
            continue;
        }
        if (digit != '\0' || text[i] < '1' || text[i] > '9') {
            return false;
        }
        digit = text[i];
    }
    if (digit == '\0') {
        return false;
    }
    result = {static_cast<unsigned>(digit - '0')};
    return true;
}
