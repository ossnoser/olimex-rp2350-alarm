# Development references

This document records the primary references collected for the hardware, SDK, tools, and framework used by the project.

## Hardware references

### RP2350-PICO2-HDR / Pico 2 family

- Olimex `RP2350-PICO2` product page: <https://www.olimex.com/Products/RaspberryPi/PICO/RP2350-PICO2/open-source-hardware>
- Raspberry Pi Pico 2 family documentation: <https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html#pico-2-family>
- Raspberry Pi Getting Started with Pico-series guide: <https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf>
- Raspberry Pi Pico 2 pinout: <https://datasheets.raspberrypi.com/pico/Pico-2-Pinout.pdf>
- Raspberry Pi Pico 2 datasheet: <https://datasheets.raspberrypi.com/pico/pico-2-datasheet.pdf>
- RP2350 datasheet: <https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf>
- RP2350 silicon documentation: <https://www.raspberrypi.com/documentation/microcontrollers/silicon.html#rp2350>

Relevant notes from the Olimex page:

- The product variants include `RP2350-PICO2` and `RP2350-PICO2-HDR`.
- The board exposes Pico 2 / RP2350 functionality in the Raspberry Pi Pico-compatible form factor.
- The page links to Raspberry Pi official Pico 2 and RP2350 documentation.

### Olimex PICO-EVB

- Olimex `PICO-EVB` product page: <https://www.olimex.com/Products/RaspberryPi/PICO/PICO-EVB/open-source-hardware>
- User manual: <https://github.com/OLIMEX/PICO-EVB/blob/main/DOCUMENTS/PICO-EVB-user-manual.pdf>
- Schematic PDF: <https://github.com/OLIMEX/PICO-EVB/blob/main/HARDWARE/PICO-EVB_Rev_A/PICO-EVB_Rev_A.pdf>
- Dimensions PDF: <https://github.com/OLIMEX/PICO-EVB/blob/main/DOCUMENTS/PICO-EVB-dimensions.pdf>
- KiCad source repository: <https://github.com/OLIMEX/PICO-EVB>

Relevant notes from the Olimex page:

- `PICO-EVB` can be used with Raspberry `PICO`, `PICO2`, `PICO-W`, `PICO2-W`, and Olimex `PICO30`.
- It provides four optoisolated inputs.
- It provides four relays with status LEDs.
- It provides a UEXT connector with `I2C`, `UART`, and `SPI` signals.
- It provides screw terminals for relay signals.
- Board dimensions are listed as `100 x 70 mm`.

### Olimex MOD-RS485

- Olimex `MOD-RS485` product page: <https://www.olimex.com/Products/Modules/Interface/MOD-RS485/open-source-hardware>
- User manual: <https://www.olimex.com/Products/Modules/Interface/MOD-RS485/resources/MOD-RS485.pdf>
- Schematic PDF: <https://www.olimex.com/Products/Modules/Interface/MOD-RS485/resources/MOD-RS485-schematic.pdf>

Relevant notes from the Olimex page:

- `MOD-RS485` is an RS485/RS422 converter module with UEXT.
- It uses the UART interface plus two control pins.
- It is based on the `ADM3483ARZ` low-power differential line transceiver.
- It operates from `3.3 V`.
- It supports half-duplex communication.
- The source page lists data rates up to `250 kbps`.

## SDK and firmware development references

### Raspberry Pi Pico SDK

- Pico SDK GitHub repository: <https://github.com/raspberrypi/pico-sdk>
- Pico C/C++ SDK documentation entry point: <https://www.raspberrypi.com/documentation/pico-sdk/>
- Getting Started with Pico-series guide: <https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf>

Relevant notes from the Pico SDK README:

- The SDK provides headers, libraries, and build system support for RP-series devices including Raspberry Pi Pico and Raspberry Pi Pico 2.
- It supports programs written in C, C++, or assembly language.
- It uses a CMake-based build system.
- It supports standard C/C++ libraries and Pico SDK hardware APIs.

### Raspberry Pi Pico VS Code extension

- Pico VS Code extension repository: <https://github.com/raspberrypi/pico-vscode>
- Releases: <https://github.com/raspberrypi/pico-vscode/releases>

Relevant notes from the extension README:

- It is the official Visual Studio Code extension for Raspberry Pi Pico development.
- It uses the official Pico SDK.
- The project generator targets the Ninja build system.
- The extension documentation points to the official Getting Started guide.

### picotool

- `picotool` repository: <https://github.com/raspberrypi/picotool>

Relevant notes from the `picotool` README:

- `picotool` works with RP2040/RP2350 binaries.
- It can interact with RP2040/RP2350 devices in BOOTSEL mode.
- It can inspect, load, save, and verify compatible firmware images.

### OpenOCD / SWD debugging

- Raspberry Pi OpenOCD repository: <https://github.com/raspberrypi/openocd>
- General OpenOCD project: <https://openocd.org/>

Relevant notes:

- OpenOCD provides on-chip programming and debugging support.
- It can act as a GDB server for source-level debugging.
- RP2350 compatibility and the exact OpenOCD configuration must be confirmed before selecting an SWD workflow.

## Host tools references

- Visual Studio Code: <https://code.visualstudio.com/>
- CMake: <https://cmake.org/>
- Ninja: <https://ninja-build.org/>
- Git: <https://git-scm.com/>
- Python: <https://www.python.org/>
- ARM GNU Toolchain downloads: <https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>
- Homebrew: <https://brew.sh/>

## Application framework references

### Quantum Leaps qpcpp

- `qpcpp` repository: <https://github.com/QuantumLeaps/qpcpp>
- QP/C++ history: <https://www.state-machine.com/qpcpp/history.html>
- Quantum Leaps official site: <https://www.state-machine.com/>

Relevant notes from the `qpcpp` README:

- The repository is for QP/C++.
- The README recommends cloning with submodules when using the GitHub repository:

```bash
git clone https://github.com/QuantumLeaps/qpcpp --recurse-submodules --depth 1
```

- The README also mentions the QP-bundle as the easiest/recommended way to get started with QP/C++.
- Technical support is handled through official SourceForge channels, not GitHub Issues.

## References still to inspect in detail

Before implementation, the following documents should be read in detail:

- `RP2350-PICO2-HDR` official schematic/pinout, if available separately from the Pico 2 references.
- `PICO-EVB` user manual and schematic.
- `MOD-RS485` user manual and schematic.
- Pico SDK documentation sections for CMake, board selection, Pico 2/RP2350 support, UART, GPIO, timers, multicore, and USB.
- `qpcpp` documentation for supported ports, build integration, and bare-metal execution models.
