// SPDX-License-Identifier: GPL-3.0-or-later
#include "command_parser.hpp"

bool Commands::parse(char const *text, std::size_t length, Timing &result) {
    char significant[3]{};
    std::size_t count = 0;
    for (std::size_t i = 0; i < length; ++i) {
        if (text[i] == ' ') {
            continue;
        }
        if (count == sizeof significant) {
            return false;
        }
        significant[count++] = text[i];
    }
    if (count != 3 || significant[0] < '1' || significant[0] > '5'
        || significant[1] != ',' || significant[2] < '1' || significant[2] > '5') {
        return false;
    }
    result = {static_cast<unsigned>(significant[0] - '0'),
              static_cast<unsigned>(significant[2] - '0')};
    return true;
}
