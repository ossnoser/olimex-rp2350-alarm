# QP/C++ integration guide

## Current implementation: standalone Pico 2 W Blinky

TASK-0013 implements the approved TASK-0012 plan. The current proof of concept targets a standalone Raspberry Pi Pico 2 W, not the earlier Olimex/carrier baseline described below. Non-wireless Pico 2 and carrier support are deferred.

**Status:** Debug build verified and standalone Pico 2 W hardware smoke test passed. With explicit user authorization, the agent copied the UF2 onto the RP2350 BOOTSEL volume; the volume disconnected after copying. The user confirmed continuous onboard LED blinking at approximately 500 ms on / 500 ms off. Cadence was visually observed, not instrument-measured; release-build, stress, and fault-injection tests remain outside this validation.

Dependencies: Pico SDK 2.3.0 (existing external installation), Arm GNU Toolchain 14.2.Rel1, and QP/C++ v8.1.5 pinned to `198b47e7169e27f41d761868fbe83e08d624cd5d`.

```bash
source env.sh
git submodule update --init third_party/qpcpp
cmake -S . -B build/pico2_w -G Ninja \
  -DPICO_BOARD=pico2_w -DPICO_PLATFORM=rp2350-arm-s \
  -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1
cmake --build build/pico2_w
```

Artifacts:

- `build/pico2_w/examples/blinky/qpcpp-blinky.elf`
- `build/pico2_w/examples/blinky/qpcpp-blinky.uf2`

The BSP API is in `bsp/include/bsp.hpp`; Pico 2 W implementation and QP hooks are in `bsp/pico2_w/`. Application state-machine code is in `examples/blinky/`; framework configuration and the explicit QV/GNU Arm static-library build are in `framework/qpcpp/`. Upstream framework files are unchanged. Blinky starts with the LED off, then changes state every 500 ms through a QP time event driven by a 100 Hz SDK interrupt timer. Core 0 runs QV with PRIMASK protection; core 1 is unused. No networking, stdio initialization, or QS tracing is enabled.

On a QP assertion or checked initialization failure, inspect `faultSource` and `faultIdentifier` in `bsp/pico2_w/qpcpp_hooks.cpp` with a debugger. The fault handler disables interrupts and halts without resetting.

For a user-coordinated hardware test: hold BOOTSEL while connecting the standalone board over USB, release BOOTSEL once the boot volume appears, and copy the generated UF2 to that volume. Confirm continuous half-second LED state changes after reboot. Record the actual observation before declaring hardware validation complete.

New application, BSP, and integration code uses GPL-3.0-or-later; the license text is in `LICENSES/GPL-3.0-or-later.txt`. Existing content and Pico SDK dependencies retain their own licensing; preserve all applicable notices and satisfy source/distribution obligations when distributing firmware.

## Earlier planning baseline

The remaining sections preserve the earlier TASK-0011 planning context. Where they describe unresolved choices or the Olimex target, the individually approved TASK-0012/TASK-0013 decisions and the current implementation section above govern this proof of concept.

## Purpose

This guide defines the intended approach for integrating Quantum Leaps QP/C++ (`qpcpp`) into the firmware project.

The first integration milestone is a minimal LED blinking firmware example based on `qpcpp`.

This document is a planning and implementation guide. It does not yet mean that `qpcpp` has been integrated.

## Initial integration goal

Create a working firmware example where LED blinking is controlled by a minimal QP/C++ application.

The example must demonstrate that:

- the Raspberry Pi Pico SDK builds correctly;
- C++ builds correctly for the RP2350/Pico2 target;
- `qpcpp` is integrated into the project build;
- a minimal active-object/event-driven application can run on the target;
- future alarm-node firmware can be based on `qpcpp`.

## Target hardware

Project target hardware:

- `RP2350-PICO2-HDR`;
- always mounted on Olimex `PICO-EVB`;
- Olimex `MOD-RS485` is part of the node baseline, but is not needed for the blinking demo.

Initial board support assumption:

- use Pico SDK `PICO_BOARD=pico2` unless a future task confirms that a custom board definition is required.

## Development environment assumptions

The initial integration assumes the verified development environment documented in:

- [Setup verification](setup-verification.md)

Baseline assumptions:

- macOS on Intel processor;
- Visual Studio Code;
- CMake;
- Ninja;
- Arm GNU Toolchain;
- Pico SDK `2.3.0`;
- `PICO_SDK_PATH` configured by sourcing `env.sh`;
- `picotool` available;
- C++ is available and preferred.

## qpcpp acquisition

A future implementation task must choose how `qpcpp` is added to the project.

Candidate approaches:

### Git submodule

`qpcpp` can be added as a Git submodule under a third-party directory.

Possible location:

```text
third_party/qpcpp
```

Advantages:

- exact upstream commit is tracked;
- repository history stays separate;
- update path is clear.

Disadvantages:

- submodule workflow requires extra Git commands;
- users must remember to initialize submodules.

Manual user step if selected:

```bash
git submodule update --init --recursive
```

### Vendored source snapshot

`qpcpp` can be copied into the repository as a fixed source snapshot.

Possible location:

```text
third_party/qpcpp
```

Advantages:

- repository is self-contained;
- no submodule workflow.

Disadvantages:

- larger repository;
- upstream updates must be handled manually;
- license and attribution files must be preserved.

### External checkout

`qpcpp` can remain outside this repository and be referenced by path.

Possible location:

```text
/Users/ossnoser/Documents/prj/quantum-leaps/qpcpp
```

Advantages:

- project repository remains small;
- easy to inspect or update independently.

Disadvantages:

- less reproducible;
- requires local setup;
- build depends on external path configuration.

## qpcpp version selection

A future implementation task must select an exact `qpcpp` version.

Candidate selection methods:

- latest stable release tag;
- fixed upstream commit;
- source snapshot from a known release bundle.

The selected version must be documented before implementation.

The upstream repository is:

- <https://github.com/QuantumLeaps/qpcpp>

The upstream README recommends cloning the GitHub repository with submodules when using it directly:

```bash
git clone https://github.com/QuantumLeaps/qpcpp --recurse-submodules --depth 1
```

## Proposed repository layout

A future implementation task should consider this layout:

```text
olimex-rp2350-alarm/
├── CMakeLists.txt
├── cmake/
│   └── ...
├── firmware/
│   ├── CMakeLists.txt
│   ├── include/
│   └── src/
├── examples/
│   └── qpcpp-blinky/
│       ├── CMakeLists.txt
│       └── src/
├── third_party/
│   └── qpcpp/
└── docs/
```

Possible minimal first implementation:

```text
examples/qpcpp-blinky/
```

The final structure must be approved by a future task before files are created.

## Minimal firmware architecture

The initial `qpcpp` blinking example should contain only the minimum required application logic.

Proposed model:

- one active object named `Blinky`;
- one periodic time event;
- one state machine that toggles an LED on each timeout;
- no alarm logic;
- no RS485 logic;
- no persistent configuration;
- no multitasking beyond what is required by the selected QP/C++ execution model.

Conceptual behavior:

```text
startup
  -> initialize Pico SDK
  -> initialize board GPIO
  -> initialize QP/C++ framework
  -> start Blinky active object
  -> dispatch periodic timeout events
  -> toggle LED
```

## Pico SDK integration points

The future implementation must define:

- top-level CMake project setup;
- inclusion of `pico_sdk_init()`;
- target board selection, initially `pico2`;
- C++ language standard;
- `pico_stdlib` linkage;
- LED GPIO configuration;
- UF2 generation;
- optional USB or UART stdio configuration.

The first build should produce at least:

```text
qpcpp-blinky.elf
qpcpp-blinky.uf2
```

## qpcpp integration points

The future implementation must identify:

- required QP/C++ include paths;
- required QP/C++ source files;
- required port layer for the selected execution model;
- minimal QP/C++ configuration header;
- startup sequence;
- assertion/error handling behavior;
- time event tick source;
- whether interrupts are needed for the first demo.

Open framework choices:

- selected QP/C++ kernel or execution model;
- cooperative versus preemptive behavior;
- bare-metal integration strategy;
- whether to use QV, QK, or another QP-supported option if applicable;
- whether tracing/logging is disabled for the first demo.

## Manual user steps

The implementation should minimize manual steps, but the guide must account for them if needed.

Possible manual steps:

1. Source the project environment:

```bash
cd /Users/ossnoser/Documents/prj/olimex-rp2350-alarm
source env.sh
```

2. Initialize submodules, if `qpcpp` is integrated as a Git submodule:

```bash
git submodule update --init --recursive
```

3. Put the board in BOOTSEL mode for UF2 flashing.

4. Copy the generated `.uf2` file to the mounted mass-storage device.

5. Optionally inspect the binary with `picotool`.

## Build instructions to define

The future implementation task must define exact commands.

Expected command style:

```bash
cd /Users/ossnoser/Documents/prj/olimex-rp2350-alarm
source env.sh
cmake -S . -B build -G Ninja -DPICO_BOARD=pico2
cmake --build build
```

Expected output artifact:

```text
build/.../qpcpp-blinky.uf2
```

The exact output path depends on the final repository layout.

## Flash/run instructions to define

Candidate UF2 workflow:

1. Hold the BOOTSEL button while connecting the board over USB.
2. Wait for the board to appear as a USB mass-storage device.
3. Copy the generated `.uf2` file to the device.
4. The board reboots automatically.
5. Confirm that the LED blinks.

Candidate `picotool` workflow:

```bash
picotool load path/to/qpcpp-blinky.uf2
picotool reboot
```

The final flashing method must be verified on the actual `RP2350-PICO2-HDR` mounted on `PICO-EVB`.

## Acceptance criteria for future implementation

A future QP/C++ integration task should be considered complete only when:

- the project configures with CMake;
- the project builds with Ninja;
- C++ compilation is used;
- `qpcpp` is part of the build;
- a `.uf2` firmware artifact is generated;
- the firmware flashes onto the target board;
- the LED blinks under QP/C++ active-object control;
- the integration method and selected `qpcpp` version are documented;
- all implementation changes are committed with an approved commit message.

## Open points before implementation

The following points must be resolved before or during the implementation task:

- Exact LED GPIO for `RP2350-PICO2-HDR` when mounted on `PICO-EVB`.
- Whether the Pico SDK `pico2` board definition is sufficient.
- Exact `qpcpp` version or commit to use.
- `qpcpp` integration method: submodule, vendored source, or external checkout.
- Selected QP/C++ port and execution model.
- Minimal QP/C++ configuration required for RP2350/Pico SDK.
- Whether the first demo should use USB stdio, UART stdio, or no stdio.
- Whether VS Code CMake configuration files should be created in the same implementation task.

## Recommended next task

Recommended next implementation task:

```text
TASK-0012 - Implement QP/C++ blinky proof of concept
```

Expected scope:

- select `qpcpp` integration method and version;
- create the initial firmware/CMake project skeleton;
- integrate the minimal QP/C++ files/configuration;
- implement `qpcpp`-based LED blinking;
- build and verify `.uf2` generation;
- document manual flashing steps and test result.
