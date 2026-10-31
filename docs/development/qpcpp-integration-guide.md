# QP/C++ integration guide

## Current board selection (TASK-0020)

Both the handwritten and QM-generated examples support **Pico 2 (default)** and **Pico 2 W** on `rp2350-arm-s`. Official Pico 2 and Olimex RP2350-PICO2-HDR use the same `pico2` software target per the user-confirmed baseline. Application/model behavior is unchanged; the BSP selects direct GPIO LED support or CYW43 LED support and shares USB/tick/framework hooks under `bsp/common/`.

Use separate `build/pico2/` and `build/pico2_w/` directories. Each builds both `qpcpp-blinky` and `qpcpp-blinky-qm`, producing four board/example combinations. New configurations default to Pico 2; existing wireless caches retain their selection. The USB product names are `Pico 2 Blinky` and `Pico 2 W Blinky`, respectively.

**Status:** all four images build, original/clone host tests and freshness checks pass, and three targeted BSP tests pass. All four board/example combinations passed separately authorized post-refactor hardware smoke tests for USB identity/enumeration, console/LED behavior, invalid-input preservation, reconnect retention and reboot defaults. TASK-0020 is complete; timing was visually observed rather than instrument-measured. Earlier milestone hardware observations below predate this refactor. No automatic flashing or commit is authorized.

See [board support and exact build/terminal commands](board-support.md) for current instructions and the artifact matrix; earlier single-board commands below preserve milestone context.

## QM-modeled clone (TASK-0019)

`examples/blinky-qm/` is an independent clone of the current duty-cycle example. Both QActive/QHsm state machines are defined in `model/blinky.qm` and generated using Intel-macOS QM 5.3.0, with QP/C++ still pinned at v8.1.5. Handwritten parser/session helpers are independent copies; BSP/framework integration is shared. The original `examples/blinky/` sources are unchanged.

**Status:** both firmware targets build, all six clone host/generator checks and the original four host tests pass, and both diagrams were exported/inspected. The clone was subsequently flashed with explicit approval. The user confirmed prompt/echo, representative duty cycles, invalid-input preservation, editing, reconnect retention and reboot defaults. USB serial enumerated after flashing and a normal power cycle. This is hardware smoke validation, not instrumented timing or USB stress testing.

The normal build uses generated C++ without requiring QM and produces separate images:

- Original: `build/pico2_w/examples/blinky/qpcpp-blinky.uf2`
- Modeled clone: `build/pico2_w/examples/blinky-qm/qpcpp-blinky-qm.uf2`

The clone uses the same duty-cycle protocol documented below. To open both diagrams in QM, edit the model, regenerate/check source, run tests or select the separate build target, follow [the clone README](../../examples/blinky-qm/README.md). Generated C++ must not be edited manually. Flashing the clone requires explicit approval and replaces the running board image, not the original example sources.

## Current example: duty-cycle commands (TASK-0016)

**Status:** completed with Debug firmware build, all four sanitizer-enabled host tests, and user-confirmed hardware smoke validation. Device checks passed for 10%/50%/90% settings, invalid-input preservation, terminal reconnect retaining 90%, reboot restoring 50%, and the prompt after USB re-enumeration. The device path changed after an upstream hub/adapter power cycle; always enumerate `/dev/cu.usbmodem*` rather than assuming a previous path. Timing was visually observed; all nine exact phase lengths were verified in host simulation, not instrumented on the board. This replaces the TASK-0015 off/on-pair protocol; earlier sections below are historical.

Connect the normally booted Pico 2 W from macOS:

```bash
ls /dev/cu.usbmodem*
screen /dev/cu.usbmodemYOUR_DEVICE 115200
```

Use the actual device path. Keep terminal local echo disabled; the device echoes typed input. If Ctrl-A followed by `\\` and `y` does not close `screen`, use a second Terminal: `screen -ls`, then `screen -S SESSION_NAME -X quit` for the specific Pico session.

The prompt is:

```text
Enter duty cycle (1..9, 10%..90% on):
```

Enter one digit and press Enter. Ordinary spaces are ignored. Valid input prints `Accepted`, invalid input prints `Invalid`, and both are followed by another prompt. Input such as `0`, `10`, `01`, `1,5`, signs, decimals, tabs, and extra characters is invalid.

| Input | LED on | LED off | Stable period |
|---|---|---|---|
| `1` | 100 ms | 900 ms | 1 second |
| `2` | 200 ms | 800 ms | 1 second |
| `3` | 300 ms | 700 ms | 1 second |
| `4` | 400 ms | 600 ms | 1 second |
| `5` | 500 ms | 500 ms | 1 second |
| `6` | 600 ms | 400 ms | 1 second |
| `7` | 700 ms | 300 ms | 1 second |
| `8` | 800 ms | 200 ms | 1 second |
| `9` | 900 ms | 100 ms | 1 second |

Boot defaults to 50%, initially off, without waiting for a terminal. Accepted updates do not restart the running phase: the next entered state uses the new duty cycle. A transition cycle can therefore contain an old and a new duration; once settings are stable, each complete on/off cycle is one second. Latest accepted input wins; invalid input leaves settings unchanged. Closing the terminal discards unfinished input but preserves the duty cycle. Reboot restores 50%.

CR/LF/CRLF, device echo, BS/DEL editing, the 32-character line bound, overflow/control recovery, fixed transmit buffering, and event-driven USB servicing are unchanged. Blinky uses nine immutable command events instead of 25 timing-pair events. Application files remain separate from the unchanged BSP transport. The 100 Hz tick provides exact 10% increments for the one-second period.

Build/test commands and artifact paths below remain unchanged. Host tests now cover all nine valid digits and actual on/off durations using the real QP/QV scheduler, including stable one-second periods, phase preservation, queued-timeout precedence, latest-command-wins, framing/editing, buffering, reconnect, no-terminal startup, and coalesced USB service. These do not replace device timing/USB tests.

For repeating the board test (obtain authorization before flashing): flash the new UF2, check the new prompt and 50% boot default, try `1`, `5`, and `9`, and verify short/equal/long on durations with a one-second steady period. Try `0`, `10`, and `1,5`; each must be invalid without changing settings. Check terminal reconnect retains the setting and a board reboot restores 50%. Record visual timing separately from instrument measurements.

## Previous example: interactive off/on timings (TASK-0015)

**Status:** completed with embedded Debug build, all four sanitizer-enabled host tests, and user-confirmed hardware smoke validation. Board checks covered prompt/echo, accepted asymmetric timings, invalid values, spaces, leading zeros, blank lines, editing, overflow/Tab rejection and recovery, pasted-command ordering, and terminal reconnection preserving settings while discarding partial input. After the USB-service follow-up, prompt/accept/reject/timing were rechecked and enumeration succeeded immediately after flashing and on two normal cold starts. Explicit line-ending variants, exact timeout races and simulated backpressure were validated on the host rather than instrumented on the board. This supersedes the output-only TASK-0014 protocol described below.

The Pico 2 W boots with 0.5 seconds off / 0.5 seconds on. Connect a Mac terminal to enter timings:

```bash
ls /dev/cu.usbmodem*
screen /dev/cu.usbmodemYOUR_DEVICE 115200
```

Use the actual enumerated device path. The baud-rate argument is conventional for USB CDC; do not enable terminal local echo because the device echoes input. Exit with Ctrl-A, then **\\**, then **y**. If that prefix does not work, open a second Mac Terminal, run `screen -ls`, identify the Pico session, and close only that session with `screen -S SESSION_NAME -X quit`.

On connection:

```text
Enter seconds off,on (1..5):
```

Enter, for example, `1,5` followed by Enter: one second off and five seconds on. The device replies `Accepted` and prompts again. `0,6` produces `Invalid` and another prompt, without changing settings. `Blink!` is no longer printed.

Protocol rules:

- Ignore ordinary spaces; require exactly one digit 1..5, comma, one digit 1..5. No signs, leading zeros, decimals, tabs, or extra characters.
- Accept CR, LF, or CRLF as one line ending. Blank lines are invalid.
- BS/DEL edit the preceding character, including spaces, and update the echo.
- A line may contain at most 32 characters including spaces, excluding Enter. Overflow or unsupported controls invalidate the entire line through Enter; only a new line recovers.
- The current LED phase completes unchanged. Accepted values apply at the next transition; the latest accepted pair wins. A configuration event takes precedence over a timeout that is queued but not dispatched.
- Closing the terminal discards partial input/pending output, not accepted settings. Reopening gives a fresh prompt. Board reboot restores 0.5 s / 0.5 s defaults.

Architecture: Blinky (priority 2) and Console (priority 1) run cooperatively on core 0. Console polls input every 20 ms and yields after at most one submission. TinyUSB event notifications also post a coalesced Console service event for prompt USB-stack handling; these events do not parse input or change the input-poll budget. All USB-stack servicing stays in application context. Per poll it processes at most 16 input bytes and offers at most 64 output bytes. It uses a 256-byte output ring and pauses input consumption before output capacity is exhausted. Each active object has an eight-entry queue. A pure parser and Console session core have no SDK/QP dependencies; the active-object adapter and LED state machine remain in `examples/blinky/`. The BSP owns TinyUSB CDC, including descriptors, session tracking and actual-byte-count nonblocking I/O. USB stdio, UART stdio, networking and QS are disabled. The BSP uses the Pico SDK's default CDC development VID/PID, not a production-specific allocation.

### Build and automated tests

Embedded build (after `source env.sh`):

```bash
cmake -S . -B build/pico2_w -G Ninja \
  -DPICO_BOARD=pico2_w -DPICO_PLATFORM=rp2350-arm-s \
  -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1
cmake --build build/pico2_w
```

Artifacts remain `build/pico2_w/examples/blinky/qpcpp-blinky.elf` and `.uf2`.

Native Mac tests, independent of the cross-compilation build:

```bash
cmake -S tests/host -B build/host -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host
ctest --test-dir build/host --output-on-failure
```

Address/undefined-behavior sanitizers default to enabled on Clang/GNU. The parser/Console suite covers grammar, editing, framing, buffering, partial writes, bounded work, and reconnection with a mock transport. The timing suite runs production Blinky/Console with the real QP core/QV scheduler, a single-threaded host critical-section shim and a mock BSP. It covers startup without a terminal, phase preservation, event precedence and command ordering, but not the Arm port or USB hardware.

### Hardware validation checklist

1. Flash the updated UF2 via BOOTSEL only after authorization. Confirm default blinking before opening a terminal.
2. Open `screen`, confirm one prompt, device echo/editing, and no `Blink!` messages.
3. Submit `1,5`, then `5,1`; check `Accepted` and off/on ordering and cadence. Change values mid-phase and verify it completes before the new duration applies.
4. Submit `0,6`, `7,8`, blank input, leading zeros, an overlong line, and unsupported controls; check `Invalid` and recovery on the next valid line.
5. Test CR/LF/CRLF if the terminal permits selecting them; each submission should yield one response/prompt.
6. Paste several short commands, confirm each response and that the latest accepted pair wins without restarting a phase.
7. Close/reopen with partial input; verify settings persist and a fresh prompt appears without stale commands/output. Reboot and confirm defaults return.

The user confirmed the board checks listed in the status above. The first post-UF2 boot did not enumerate USB; a normal physical USB reconnect enumerated the device successfully. A later reboot restored the default LED cadence, as confirmed by the user, but USB enumeration failed again. The approved follow-up supplements polling with event-driven USB servicing. After reflashing, USB enumerated immediately and on two normal unplug/reconnect cycles, with user-confirmed console operation and restored default timings. The failure has not recurred in these smoke checks; its exact root cause and long-term reliability are not established. Visual timing checks are not instrument measurements. TinyUSB servicing is nonwaiting but internally drains queued USB events; no hard real-time or hostile-host execution bound is claimed. Fixed buffers cannot preserve arbitrarily large pastes if a host ignores backpressure.

## Historical milestones

The sections below record earlier implementations and their actual hardware validation; their output-only behavior does not describe TASK-0015.

## USB serial extension (TASK-0014)

**Status:** Debug build and hardware smoke test passed. The user confirmed `Blink!` output with LED blinking, continued blinking while the terminal was closed, and output resuming once per second without a backlog burst after reopening it. Timing was visually observed, not instrument-measured. The previously validated LED-only firmware is described below.

Blinky now calls `BSP::writeLine("Blink!")` immediately after entering LED-on. The BSP sends `Blink!\r\n` over USB CDC, with UART and QS still disabled. USB initialization never waits for a terminal. Output is best effort, with zero USB-buffer and stdio-lock wait timeouts and no explicit blocking flush. Disconnected messages are skipped; there is no application replay queue. This is not a hard real-time guarantee: the USB stack still needs processing time, and congestion/disconnection can drop output, including partial lines.

Build using the commands below; the ELF/UF2 paths are unchanged. The build was verified with SDK 2.3.0 and the approved Debug settings. Compilation commands for BSP and SDK stdio sources were checked for consistent zero-wait settings, USB enabled, and UART disabled.

### Connect a Mac terminal

After explicitly authorizing and flashing the updated UF2, leave the board connected over a data-capable USB cable in normal firmware mode (not BOOTSEL).

1. Open Terminal on the Mac and find the USB CDC device:

   ```bash
   ls /dev/cu.usbmodem*
   ```

2. Select the device belonging to the Pico 2 W (disconnect/reconnect to identify it if several appear), then open it using its actual path:

   ```bash
   screen /dev/cu.usbmodemYOUR_DEVICE 115200
   ```

   The speed argument is required by `screen`; USB CDC output timing is not determined by this baud rate. Normal terminal opening asserts DTR, which the SDK uses to recognize a connected terminal.

3. Expect one `Blink!` line each time the LED turns on, approximately once per second. No startup banner or backlog is sent.
4. Exit with **Ctrl-A**, then **\\**, then confirm with **y**. Only one program should own the serial device at a time.
5. Confirm the LED keeps blinking with the terminal closed, then reopen it and check that current messages resume without an application backlog.

If the device is absent, verify the updated firmware is flashed and the board is not in BOOTSEL mode. If it is busy, close other terminal/serial-monitor sessions. The device path may change after reboot or reconnect; enumerate it again.

Flashing, output/blinking observation, and terminal-close/reopen behavior passed with user confirmation. USB saturation/backpressure timing has not been stress-tested.

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
