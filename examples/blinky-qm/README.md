# Blinky with QM-generated state machines

Independent clone of `examples/blinky/`, with **both active-object state machines defined in QM**. The original example is unchanged. The BSP and QP/C++ integration are shared, not copied.

**Current board support (TASK-0020):** Pico 2 (default) and Pico 2 W are supported. Both examples build for both boards; the clone's four sanitizer-enabled behavior tests and two optional generator checks, the original four host tests, and three BSP adapter tests pass. Post-refactor hardware smoke tests passed on both boards for USB identity/enumeration, representative duty cycles, invalid-input preservation, reconnect retention and reboot defaults. Timing was visually observed, not instrument-measured. See [board selection and build matrix](../../docs/development/board-support.md).

**Earlier TASK-0019 validation:** both diagrams were exported/inspected and the Pico 2 W clone was flashed with explicit approval. The user confirmed the prompt, representative duty cycles, invalid-input preservation, editing, terminal retention and reboot defaults. Those visually observed smoke tests predate the dual-board BSP refactor.

## Behavior

The behavior matches the original duty-cycle example:

- Standalone Pico 2 or Pico 2 W, core 0, cooperative QV; QP/C++ pinned at v8.1.5.
- One-second stable LED cycle. Input `1`..`9` selects 10%..90% on-time; boot defaults to 50%, initially off.
- USB prompt `Enter duty cycle (1..9, 10%..90% on):`, device-side echo/editing, and `Accepted`/`Invalid` followed by another prompt.
- Accepted input updates the next phase without restarting the current timer. The latest accepted value wins; invalid input preserves settings.
- Ordinary spaces ignored; only a single digit 1..9 is accepted. CR/LF/CRLF, BS/DEL, the 32-character line bound and recovery are unchanged.
- Closing the terminal discards partial input/output but retains duty cycle. Reboot restores 50%.
- Bounded, buffered nonblocking console I/O, with USB servicing driven by coalesced QP events as well as polling. No UART, networking or QS.

## Source ownership

```text
model/blinky.qm             Source of truth for both machines
generated/                 Generated class declarations/definitions; never hand-edit
src/                       Startup, queue/event/USB adapters and copied parser/session helpers
tools/qm_codegen.py         Explicit regeneration, freshness and diagram export
tests/                     Independent clone host tests and generator-check tests
```

The model includes constructors, time-event initialization, state hierarchy, entry actions and event/transition handlers:

- **BlinkyAO:** initial -> `running.off`; TIMEOUT alternates `off`/`on`. Parent `running` handles DUTY by updating future phase lengths, without rearming the current phase.
- **ConsoleAO:** initial -> `active`; POLL runs the bounded session helper. USB_SERVICE releases the coalescing flag and services USB without parsing input.

No handwritten `Q_STATE_DEF`/transition implementation exists in `src/`. Queue storage, immutable command events, ISR notification coalescing and transport adapters are handwritten support, not alternative state-machine definitions. The parser and console-session helper copies are deliberately independent of the original example.

## Open and edit in QM

Use **QM 5.3.0 for Intel macOS**, already checked against QP/C++ v8.1.5. Standalone QMC is not needed; the installed QM application supports command-line generation.

From the repository root:

```bash
open -a /Applications/qm.app examples/blinky-qm/model/blinky.qm
```

In QM's Model Explorer expand **AOs**, then **BlinkyAO -> SM** or **ConsoleAO -> SM**. Open the statechart to edit states, transitions and actions. Brief diagram labels summarize some actions; select the corresponding item to inspect its full C++ action in the property sheet. Constructors/members are also modeled under each class. Do not edit `generated/` to change behavior.

Save the model, then regenerate and check:

```bash
python3 examples/blinky-qm/tools/qm_codegen.py regenerate
python3 examples/blinky-qm/tools/qm_codegen.py check
```

Review and commit model plus generated changes together when a commit is authorized. A normal firmware build intentionally does **not** auto-regenerate: forgetting this step can otherwise build stale behavior. The freshness check detects that mismatch.

Generation tools require Python 3.9+ and verify the exact `qm 5.3.0` banner. Override the default executable with either:

```bash
python3 examples/blinky-qm/tools/qm_codegen.py check --qm /path/to/qm
# or
QM_EXECUTABLE=/path/to/qm python3 examples/blinky-qm/tools/qm_codegen.py check
```

No tool is downloaded, installed or upgraded automatically. All generation/check work uses isolated directories under ignored `build/`. `check` does not overwrite generated source; `regenerate` copies generator bytes verbatim and keeps generated output read-only. Missing, changed or extra generated files fail the check.

### Export diagrams

```bash
python3 examples/blinky-qm/tools/qm_codegen.py diagrams
```

Outputs under ignored `build/blinky-qm-diagrams/`:

- `SM_of_BlinkyAO.svg` and `.png`
- `SM_of_ConsoleAO.svg` and `.png`

Use `--output PATH` for another project-local destination. Diagram layout lives in the model; exported images are derived artifacts, not a separate editable model.

## Build firmware without requiring QM

From the repository root, build the non-wireless Pico 2 default:

```bash
source env.sh
cmake -S . -B build/pico2 -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1
cmake --build build/pico2
```

For Pico 2 W, use a separate directory and explicit board selection:

```bash
cmake -S . -B build/pico2_w -G Ninja \
  -DPICO_BOARD=pico2_w -DPICO_PLATFORM=rp2350-arm-s \
  -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1
cmake --build build/pico2_w
```

Existing caches retain their selected board; do not switch board types in one build directory. For either `BOARD=pico2` or `BOARD=pico2_w`, the normal build creates two independent firmware images:

- Original: `build/BOARD/examples/blinky/qpcpp-blinky.{elf,uf2}`
- Modeled clone: `build/BOARD/examples/blinky-qm/qpcpp-blinky-qm.{elf,uf2}`

`BOARD` above is a placeholder for the actual directory name, not a CMake variable. Build just the clone, for example:

```bash
cmake --build build/pico2 --target qpcpp-blinky-qm
```

Optional explicit generation/check build targets (never dependencies of either firmware):

```bash
cmake --build build/pico2 --target blinky-qm-regenerate
cmake --build build/pico2 --target blinky-qm-check
```

Generated code is compiled with `QP_API_VERSION=815`; no old-API compatibility mode or framework downgrade is required. Board selection happens in the shared BSP; the application/model/generated code and QP configuration do not change between boards.

## Host tests

Four behavior cases run without requiring QM:

```bash
cmake -S examples/blinky-qm/tests -B build/blinky-qm-host -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DENABLE_QM_CHECKS=OFF
cmake --build build/blinky-qm-host
ctest --test-dir build/blinky-qm-host --output-on-failure
```

To also run the two generator checks using QM 5.3.0:

```bash
cmake -S examples/blinky-qm/tests -B build/blinky-qm-host -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DENABLE_QM_CHECKS=ON
cmake --build build/blinky-qm-host
ctest --test-dir build/blinky-qm-host --output-on-failure
```

AddressSanitizer/UndefinedBehaviorSanitizer default to enabled for the C++ tests on Clang/GNU. Parser/session tests cover all nine values, framing/editing, invalid input, overflow/controls, partial output/backpressure, bounded work and reconnection. Timing tests compile the **generated** classes against the real QP core/QV scheduler and test all duty-cycle durations, phase preservation, queued-timeout precedence, latest-command-wins, startup and USB-wakeup coalescing. The single-threaded host critical-section shim and mock BSP do not validate Arm interrupt behavior, physical USB enumeration or LED timing.

Generator tests verify exact regeneration and reject stale, missing, extra output and a wrong-version executable using disposable project-local fixtures; tracked generated files are not modified. The original regression tests remain available independently:

```bash
ctest --test-dir build/host --output-on-failure
```

## Board test and Mac terminal

The earlier Pico 2 W smoke test is recorded above; post-refactor tests passed for both Pico 2 and Pico 2 W. Obtain explicit authorization and confirm the physical board before copying **the clone's matching-board** `qpcpp-blinky-qm.uf2` to the RP2350 BOOTSEL volume. It replaces the running image, not the original source example. The product name is `Pico 2 Blinky` for the non-wireless target or `Pico 2 W Blinky` for the wireless target. The boot volume itself identifies only RP2350, not the board variant.

After reboot:

```bash
ls /dev/cu.usbmodem*
screen /dev/cu.usbmodemYOUR_DEVICE 115200
```

Use the actual path, which can change on reconnect. Test default 50%, `1`/`5`/`9`, invalid `0`/`10`/`1,5`, editing, latest-command-wins, terminal-close/reopen retention, and reboot defaults. Close `screen` with Ctrl-A then `\\` then `y`, or from another Terminal with `screen -ls` and `screen -S SESSION_NAME -X quit` for the Pico session. Keep local echo disabled.

Record board observations separately from host tests and distinguish visual cadence from instrument measurements. USB-stack work is nonwaiting but not claimed hard real-time or immune to arbitrary host flooding.

## License

Model, handwritten code and generated code use GPL-3.0-or-later. Preserve generator notices; see `../../LICENSES/GPL-3.0-or-later.txt`. QM itself is a separately licensed tool and is not redistributed here. Shared dependencies retain their own notices/licenses.
