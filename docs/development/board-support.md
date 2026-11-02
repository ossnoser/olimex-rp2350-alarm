# Pico 2 and Pico 2 W board support

## Supported targets

Both Blinky firmware examples support the following standalone boards:

| Pico SDK board | LED implementation | USB product name |
|---|---|---|
| `pico2` (default) | GPIO via `PICO_DEFAULT_LED_PIN` (SDK default: GP25) | `Pico 2 Blinky` |
| `pico2_w` | CYW43 LED via `pico_cyw43_arch_none`, without networking | `Pico 2 W Blinky` |

Per the user-confirmed hardware baseline, official Raspberry Pi Pico 2 and Olimex RP2350-PICO2-HDR use the same `pico2` software target; pre-mounted headers do not require a separate build profile. The Blinky examples do not initialize PICO-EVB I/O or RS485. A separate [Pico 2-only carrier diagnostic](../../examples/pico-evb-diagnostic/README.md) has its own independent QM model. Its original output-only build passed unloaded/input checks in TASK-0022..0024. TASK-0026/0027's REL1-only/four-relay sequence images passed limited approved unloaded observations. Current TASK-0028 replaces the sequence with individual latched `on N`/`off N` and global `off`, maximum one relay ON, no timeout, detected session-loss OFF and no lockout. This new manual-control image passed automated checks and approved flash/startup verification, and one approved manual REL1 ON/selected-OFF console trace; the user confirmed the physical REL1 ON-until-OFF behavior and other indicators OFF. The user also reported all-four manual control/busy rejection/global OFF successful. Unpowered/unwired cleanup was user-confirmed; no full physical failure-path/contact/current/thermal qualification is implied. Relay/load and transient behavior remain unvalidated.

Only `PICO_PLATFORM=rp2350-arm-s` is supported. RISC-V and other boards are rejected at configure time. C++17, QV/core 0, the 100 Hz framework tick, QP/C++ v8.1.5, the duty-cycle protocol and the QM model are unchanged.

**TASK-0020 status:** all four firmware combinations build; original/clone host tests, QM freshness checks, and three targeted LED-adapter tests pass. All four board/example combinations passed separately authorized post-refactor smoke tests for USB identity/enumeration, duty-cycle console/LED behavior, invalid-input preservation, terminal reconnect and reboot defaults. TASK-0020 is complete. Timing was visually observed, not instrument-measured; USB stress and fault injection were not performed on hardware.

## BSP boundaries

```text
bsp/
  include/bsp.hpp          Backward-compatible API plus optional startup hooks
  common/
    board.hpp              Internal LED initialization/product-name contract
    bsp.cpp                Shared initialization and nonblocking USB CDC transport
    qpcpp_hooks.cpp         Shared timer, QV idle and fail-stop diagnostics
    tusb_config.h          Shared USB stack configuration
    usb_descriptors.cpp    Shared descriptors, selected board/product name
    runtime_options.*      Optional fault/session-loss actions and startup USB identity
  pico2/led.cpp             GPIO LED initialization/control and identity
  pico2_w/led.cpp           CYW43 LED initialization/control and identity
  carriers/pico_evb_rev_a/  Diagnostic-only GPIO/input/silicon adapter
```

The non-wireless build has no CYW43 sources/library dependency. The SDK may report wireless libraries as *available* during configuration; that does not mean they are linked. Pico 2 W alone links the CYW43 architecture support.

USB CDC VID/PID and board-unique serial identity are unchanged. Product names reflect the selected firmware profile, not automatic hardware detection. Application code never uses board GPIO/CYW43 APIs. Both original and QM-generated examples call the same BSP interface.

## Build each board separately

From the project root:

```bash
source env.sh

# New build directories default to non-wireless Pico 2.
cmake -S . -B build/pico2 -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1
cmake --build build/pico2

# Explicitly select the wireless board in a different directory.
cmake -S . -B build/pico2_w -G Ninja \
  -DPICO_BOARD=pico2_w -DPICO_PLATFORM=rp2350-arm-s \
  -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1
cmake --build build/pico2_w
```

`-DPICO_BOARD=pico2` may be passed explicitly for the first command. The default does not override an existing CMake cache: an existing `build/pico2_w/` remains wireless. Do not switch board types inside a previously configured build directory.

Pico 2 builds now create **three separate firmware images**; Pico 2 W retains **two**. These are separate applications, not one image containing all examples:

| Board | Example | Artifact stem (append `.elf` or `.uf2`) |
|---|---|---|
| Pico 2 | Handwritten | `build/pico2/examples/blinky/qpcpp-blinky` |
| Pico 2 | QM-modeled | `build/pico2/examples/blinky-qm/qpcpp-blinky-qm` |
| Pico 2 + EVB Rev. A | Manual relay/input diagnostic | `build/pico2/examples/pico-evb-diagnostic/qpcpp-pico-evb-diagnostic` |
| Pico 2 W | Handwritten | `build/pico2_w/examples/blinky/qpcpp-blinky` |
| Pico 2 W | QM-modeled | `build/pico2_w/examples/blinky-qm/qpcpp-blinky-qm` |

Build one application explicitly with `--target qpcpp-blinky` or `--target qpcpp-blinky-qm` in the corresponding board directory. Normal builds use stored generated C++ and do not require running QM.

## Automated validation

Original example and QM clone tests:

```bash
ctest --test-dir build/host --output-on-failure
ctest --test-dir build/blinky-qm-host --output-on-failure
python3 examples/blinky-qm/tools/qm_codegen.py check
```

See the [clone README](../../examples/blinky-qm/README.md) for configuring its host tests. The six-case clone run uses `ENABLE_QM_CHECKS=ON` and installed QM 5.3.0; without generator checks, four behavior cases run independently of QM.

Targeted native BSP tests use SDK stubs, not real GPIO/radio hardware:

```bash
cmake -S tests/bsp -B build/bsp-host -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/bsp-host
ctest --test-dir build/bsp-host --output-on-failure
```

These check GPIO initialization/off-before-output sequencing, on/off control, use of the configured LED macro rather than a hardcoded pin, CYW43 initialization/failure handling and selected product identity. Address/undefined-behavior sanitizers default to enabled on Clang/GNU.

After building both board directories, inspect all four actual build graphs and images:

```bash
python3 tests/bsp/verify_builds.py
```

This checks board/platform caches, selected LED sources, shared descriptors, no standard-board CYW43 dependency, no USB/UART stdio, linked adapter symbols, expected product-name literals, and nonempty ELF/UF2 files. It does not open a USB connection or flash/run firmware.

## Hardware test and Mac terminal

Obtain explicit approval for each flash. Confirm the **physical board variant** and connect only the board being tested, then enter BOOTSEL. Its RP2350 boot-volume identity does not distinguish Pico 2 from Pico 2 W. Copy the matching board/example UF2 from the matrix above; do not infer hardware type from a firmware product string.

After reboot, enumerate the actual serial port:

```bash
ls /dev/cu.usbmodem*
screen /dev/cu.usbmodemYOUR_DEVICE 115200
```

The path may change after USB hub/power changes. Check the board-appropriate product name in macOS System Information if necessary. Keep terminal local echo disabled. Close `screen` with Ctrl-A, then `\\`, then `y`, or use a second Terminal: `screen -ls` followed by `screen -S SESSION_NAME -X quit` for the Pico session.

For each board/example combination, check:

1. Default 50% LED duty cycle without a terminal and successful USB enumeration.
2. Correct prompt; `1`, `5`, `9` accepted with 10%/50%/90% on-time over a one-second stable cycle.
3. Invalid `0`/`10`/`1,5` preserve the selected setting.
4. Echo/editing and terminal reopen retain settings and show a fresh prompt.
5. A normal USB power cycle restores 50% and re-enumerates USB.

These five duty-cycle smoke checks apply to the **two Blinky examples**, not the output-only carrier diagnostic. Follow its separately gated README instead; it exposes no duty/relay commands. Carrier initialization and fault actions are linked/configured only by that diagnostic, which uses the USB product name `Pico 2 EVB Diagnostic`.

Record each physical combination separately. Visual cadence observations are not instrument measurements. Build/host tests do not prove board pin wiring, USB enumeration or Arm interrupt behavior. TASK-0020 post-refactor hardware status: all four board/example combinations passed the recorded smoke checks.
