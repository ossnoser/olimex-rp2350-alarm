# SPDX-License-Identifier: GPL-3.0-or-later

add_executable(qpcpp-blinky
    main.cpp blinky.cpp console.cpp console_session.cpp command_parser.cpp
)
target_link_libraries(qpcpp-blinky PRIVATE bsp)
pico_enable_stdio_usb(qpcpp-blinky 0)
pico_enable_stdio_uart(qpcpp-blinky 0)
pico_add_extra_outputs(qpcpp-blinky)
