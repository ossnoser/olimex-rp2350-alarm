# Installation baseline for macOS Intel

This document lists the software that should be installed on the macOS Intel development PC before starting firmware development.

The list is based on the references collected in [Development references](references.md).

## Required tools

These tools are required to build the initial firmware project.

### Visual Studio Code

Install Visual Studio Code for macOS Intel.

Purpose:

- main editor/IDE;
- C/C++ code editing;
- CMake project handling;
- optional flashing/debug task integration.

Reference:

- <https://code.visualstudio.com/>

### Git

Purpose:

- project version control;
- cloning Pico SDK;
- cloning or integrating `qpcpp`;
- future dependency management.

Reference:

- <https://git-scm.com/>

### CMake

Purpose:

- required by the Raspberry Pi Pico SDK build system;
- project configuration and generation of build files.

Reference:

- <https://cmake.org/>

### Ninja

Purpose:

- preferred build executor for VS Code and Pico SDK workflows;
- used by the Raspberry Pi Pico VS Code extension project generator.

Reference:

- <https://ninja-build.org/>

### Python 3

Purpose:

- required by common Pico SDK tooling and scripts;
- useful for future development utilities.

Reference:

- <https://www.python.org/>

### ARM GNU embedded toolchain

Install an ARM bare-metal toolchain that provides:

- `arm-none-eabi-gcc`;
- `arm-none-eabi-g++`;
- `arm-none-eabi-gdb`;
- `arm-none-eabi-objcopy`;
- `arm-none-eabi-size`.

Purpose:

- C compilation;
- C++ compilation;
- linking firmware for RP2350;
- binary/UF2-related build steps;
- optional GDB debugging.

Reference:

- <https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>

### Raspberry Pi Pico SDK

Install or clone the Pico SDK with RP2350/Pico2 support.

Purpose:

- headers, libraries, and CMake support for RP-series microcontrollers;
- C and C++ firmware development;
- Pico2/RP2350 hardware APIs.

Reference:

- <https://github.com/raspberrypi/pico-sdk>

Important setup note:

- The environment variable `PICO_SDK_PATH` will likely be required by the build configuration unless the SDK is integrated differently.

## Required project framework source

### Quantum Leaps qpcpp

The project intends to integrate Quantum Leaps `qpcpp`.

Purpose:

- active-object/event-driven application framework;
- state-machine-based firmware architecture.

Reference:

- <https://github.com/QuantumLeaps/qpcpp>

Expected acquisition options to decide later:

- Git submodule;
- vendored source snapshot;
- external checkout documented by setup instructions.

The upstream README recommends cloning the GitHub repository with submodules when using the repository directly:

```bash
git clone https://github.com/QuantumLeaps/qpcpp --recurse-submodules --depth 1
```

No integration method is selected yet.

## Recommended tools

These tools are recommended for an efficient workflow but may not be strictly required for the first build.

### picotool

Purpose:

- inspect RP2040/RP2350 binaries;
- interact with RP2040/RP2350 devices in BOOTSEL mode;
- load, save, and verify firmware images.

Reference:

- <https://github.com/raspberrypi/picotool>

### Raspberry Pi Pico Visual Studio Code extension

Purpose:

- streamline Pico SDK projects in VS Code;
- project generation;
- integration with Ninja and Pico SDK workflows.

Reference:

- <https://github.com/raspberrypi/pico-vscode>

Status for this project:

- recommended to evaluate;
- compatibility with the selected `RP2350-PICO2-HDR` workflow must be confirmed.

### VS Code C/C++ extension

Purpose:

- C/C++ IntelliSense;
- navigation;
- build/debug integration support.

Recommended extension:

- Microsoft C/C++.

### VS Code CMake Tools extension

Purpose:

- configure/build CMake projects from VS Code;
- select presets/kits/build variants;
- integrate with Ninja.

Recommended extension:

- Microsoft CMake Tools.

## Optional tools

These tools are optional until the corresponding workflow is selected.

### OpenOCD

Purpose:

- SWD debugging/programming;
- GDB server for source-level debugging.

References:

- <https://github.com/raspberrypi/openocd>
- <https://openocd.org/>

Status for this project:

- optional until SWD debugging is selected;
- RP2350 support and exact configuration must be confirmed.

### SWD debug probe

Candidate hardware:

- Raspberry Pi Debug Probe;
- PicoProbe;
- CMSIS-DAP compatible probe;
- J-Link compatible probe.

Status for this project:

- optional until a debug workflow is selected.

### VS Code Cortex-Debug extension

Purpose:

- GDB/OpenOCD debug sessions from VS Code.

Status for this project:

- optional until SWD debugging is selected.

### clangd

Purpose:

- alternative C/C++ language server;
- fast code navigation and diagnostics.

Status for this project:

- optional; use only if preferred over Microsoft C/C++ IntelliSense.

## Likely installation source on macOS Intel

Homebrew is a practical installation source for many host tools:

- `git`;
- `cmake`;
- `ninja`;
- `python3`;
- possibly `picotool`;
- possibly OpenOCD.

The ARM GNU embedded toolchain may be installed either from:

- official ARM packages; or
- a Homebrew package, if the selected package is current and compatible with RP2350/Pico SDK requirements.

Exact commands are intentionally deferred to a future setup task, after versions and installation methods are selected.

## Minimum install set to start development

A practical minimum set for the first firmware build is:

1. Visual Studio Code.
2. Git.
3. CMake.
4. Ninja.
5. Python 3.
6. ARM GNU embedded toolchain.
7. Raspberry Pi Pico SDK.
8. Quantum Leaps `qpcpp` source checkout or selected integration method.

## Recommended install set for first hardware flashing

Add:

1. `picotool`.
2. VS Code C/C++ extension.
3. VS Code CMake Tools extension.
4. Raspberry Pi Pico VS Code extension, if confirmed suitable.

## Recommended install set for source-level debugging

Add:

1. OpenOCD compatible with RP2350.
2. SWD debug probe.
3. VS Code Cortex-Debug extension.
4. `arm-none-eabi-gdb`, if not already included in the installed ARM toolchain.

## Open decisions before installation commands are finalized

- Select exact Pico SDK version or commit.
- Confirm the CMake board identifier for `RP2350-PICO2-HDR`.
- Confirm whether a custom board definition is required.
- Select ARM GNU toolchain installation source and version.
- Select `qpcpp` version or commit.
- Select `qpcpp` integration method.
- Decide whether SWD debugging is required immediately or later.
- Confirm whether the Raspberry Pi Pico VS Code extension will be part of the standard workflow.
