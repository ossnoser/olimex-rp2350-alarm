# Development toolchain

## Host environment

The development host is:

- macOS on Intel processor.
- Visual Studio Code as the main editor/IDE.

Tool installation should prefer macOS Intel-compatible packages. Homebrew may be used where appropriate.

## Required base tools

The following tools are required or expected for the firmware project:

- `git`: source control and dependency checkout.
- `cmake`: build system generator used by the Raspberry Pi Pico SDK.
- `ninja` or `make`: build executor. `ninja` is preferred for VS Code/CMake integration.
- `python3`: required by parts of the Pico SDK tooling and common build scripts.

## ARM embedded C/C++ toolchain

The project requires an ARM bare-metal toolchain compatible with RP2350/Pico2 development:

- `arm-none-eabi-gcc`: C compiler.
- `arm-none-eabi-g++`: C++ compiler.
- `arm-none-eabi-gdb`: debugger, if SWD debugging is used.
- `arm-none-eabi-objcopy`: binary conversion utility.
- `arm-none-eabi-size`: firmware size reporting utility.
- related ARM `binutils` components.

The toolchain must support building C and C++ firmware for ARM Cortex-M class microcontrollers.

## Pico SDK and board support

The project is expected to use the Raspberry Pi Pico SDK with support for:

- RP2350 microcontroller family;
- Raspberry Pi Pico2-compatible boards;
- C and C++ firmware builds;
- CMake-based project configuration;
- UF2 firmware image generation.

Open points:

- Confirm the exact SDK version to use.
- Confirm whether `RP2350-PICO2-HDR` requires a custom board definition.
- Confirm the final CMake board identifier for the target.

## Visual Studio Code

Required editor:

- Visual Studio Code for macOS Intel.

Recommended extensions:

- Microsoft C/C++ extension.
- CMake Tools.
- Cortex-Debug, if SWD debugging will be used.

Optional extensions:

- `clangd`, if a clangd-based code model is preferred over the Microsoft C/C++ engine.
- EditorConfig, if a future formatting policy uses `.editorconfig`.
- Raspberry Pi Pico extension, if confirmed compatible with RP2350/Pico2 and the selected workflow.

## Flashing tools

Expected or candidate tools:

- UF2 mass-storage flashing over USB boot mode.
- `picotool` for inspecting, loading, and managing Pico/Pico2 firmware images.

Open points:

- Confirm whether normal development flashing will use UF2 drag-and-drop, `picotool`, or VS Code tasks.
- Confirm the final boot/reset workflow for `RP2350-PICO2-HDR` mounted on `PICO-EVB`.

## Debug tools

SWD debugging is optional until explicitly selected.

Candidate requirements if SWD debug is used:

- OpenOCD version compatible with RP2350.
- SWD probe, for example:
  - Raspberry Pi Debug Probe;
  - PicoProbe;
  - CMSIS-DAP compatible probe;
  - J-Link compatible probe.
- VS Code Cortex-Debug configuration.

Open points:

- Confirm whether SWD debugging is required.
- Confirm the physical debug connector and wiring.
- Confirm OpenOCD support and configuration for RP2350.

## Tool installation candidates on macOS Intel

Likely installation sources include:

- Homebrew for `git`, `cmake`, `ninja`, `python3`, and possibly `picotool` or OpenOCD.
- Official ARM GNU Toolchain packages for macOS, if preferred or required.
- Official Visual Studio Code download for macOS Intel.
- Raspberry Pi Pico SDK Git repository.

Exact installation commands are intentionally not fixed yet and should be documented in a future setup task after tool versions are selected.
