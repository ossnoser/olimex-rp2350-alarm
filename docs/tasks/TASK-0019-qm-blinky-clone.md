# TASK-0019 - QM-modeled Blinky clone

## Status

Approved / completed; generation, build, host tests and hardware smoke validation passed.

The user approved the documented design and subsequently explicitly approved implementing TASK-0019. Clone sources, build integration, regeneration tools, documentation and tests within this scope are authorized. Flashing was subsequently separately authorized. After completing TASK-0020, the user also explicitly authorized committing the accumulated QM-clone and dual-board work.

## Objective

Create an independently editable clone of the current duty-cycle Blinky example under `examples/blinky-qm/`. Define both active-object state machines graphically in Quantum Leaps QM and generate their QP/C++ implementation, without changing the original example.

## Prerequisites and investigation results

- [TASK-0017](TASK-0017-qm-macos-intel-investigation.md) verified QM/QMC v5.3.0 as the newest official stable Intel macOS packages among the inspected releases. Newer inspected macOS GUI releases are ARM64 only.
- The user installed QM v5.3.0 at `/Applications/qm.app`.
- [TASK-0018](TASK-0018-qm-generated-code-compatibility.md) successfully generated two QActive/QHsm probe machines, ran them against the real QP/C++ v8.1.5 QV host core, compiled unchanged generated code with the Pico 2 W Arm flags, and exported both diagrams.
- The existing clone source baseline is the completed TASK-0016 duty-cycle example, not the older off/on-seconds protocol.

## Individually approved decisions

1. **Modeling scope:** model both Blinky and Console active-object state machines in QM. Keep the numeric parser and console-session helpers handwritten.
2. **Directory/separation:** create `examples/blinky-qm/`; leave `examples/blinky/` unchanged. Share the existing BSP and QP/framework integration.
3. **Available tool:** the initial installed QM version was 5.1.3. The user subsequently installed and selected QM 5.3.0 after the Intel-package investigation.
4. **Compatibility policy:** pin QP/C++ at v8.1.5 and verify generator compatibility rather than downgrade the working framework. The approved investigation/compatibility checks are complete; no further tool/dependency changes are implied.
5. **Implementation strategy:** retain QActive/QHsm, matching the original example and the successful compatibility probe. QM generates state handlers and transitions; do not switch to QMActive/QMsm.
6. **Source-of-truth workflow:** the `.qm` model owns both state machines. Check generated C++ into Git and never edit it manually. Normal firmware builds consume tracked generated code without requiring QM. Explicit regeneration uses QM 5.3.0; a consistency check detects stale generated files. Handwritten helpers remain separate.
7. **Independent helpers:** copy parser and console-session helpers into the clone rather than link back into the original example. Only BSP/framework integration is shared.
8. **Layout:** `CMakeLists.txt`, `README.md`, `model/blinky.qm`, `generated/`, `src/`, `tools/`, and `tests/` under the clone directory. One model contains both machines, each with its own diagram.
9. **Build:** add independent `qpcpp-blinky-qm` firmware alongside the existing `qpcpp-blinky`. A normal build produces both separate images; it does not combine four active objects in one image. Support explicit `cmake --build build/pico2_w --target qpcpp-blinky-qm`.
10. **Diagrams:** Blinky initially enters `off`; `off`/`on` are nested in `running`, which handles duty-cycle updates. Timeouts alternate LED states. Console initially enters one `active` state with internal poll and USB-service handlers. Preserve existing behavior; do not introduce artificial states or move parsing/editing into the diagrams.
11. **Generator setup:** use installed QM 5.3.0 command-line mode (`qm -c`), not a mandatory standalone QMC. Verify exact version, default to `/Applications/qm.app/Contents/MacOS/qm`, allow an explicit executable override, and never install/upgrade automatically.
12. **Validation:** run parser/Console, timing/ordering, startup, and USB-wakeup host tests against the clone's generated active objects; rerun the original four tests; check byte-identical regeneration; export/inspect both diagrams; build both firmware images. Flashing/committing remain separately gated.
13. **Planning gate:** record this plan and index entry only, then request separate implementation approval.

## Behavior to preserve

- Standalone Pico 2 W, Cortex-M33 Arm platform, C++17 and the existing Debug configuration.
- Core 0, cooperative QV, PRIMASK protection, assertions enabled; core 1 unused.
- One-second stable blink cycle, single-digit `1`..`9` selecting 10%..90% on-time. Initial state off, boot duty cycle 50%.
- Prompt `Enter duty cycle (1..9, 10%..90% on):`, `Accepted`/`Invalid` responses and repeated prompting, all with CRLF.
- Ordinary spaces ignored; reject zero, multi-digit values, leading zeros, old pair syntax, signs, decimals and extra characters.
- CR/LF/CRLF framing, device echo, BS/DEL editing, 32-character input bound, overflow/unsupported-control recovery.
- Fixed 256-byte Console transmit buffer, bounded nonblocking input/output work, eight-entry AO queues, no application heap/event pools.
- Blinky priority 2, Console priority 1, 100 Hz framework tick, 20 ms input polling.
- Nine immutable duty-cycle events. Updates take precedence over queued timeouts; Console yields after each submission. Keep the running phase unchanged and use the latest accepted duty cycle in the next state.
- BSP-managed TinyUSB CDC with event-driven/coalesced USB service wakeups. Service USB in application context; do not parse in interrupts or bypass input-poll limits.
- Reconnect discards unfinished input/output, prompts anew and retains duty cycle; reboot restores 50%.
- No `Blink!` output, UART, networking, QS, persistent storage, watchdog changes or new application behavior.

## Approved implementation scope

### Clone and graphical model

- Copy the current parser/session helpers into `src/` and retain their GPL-3.0-or-later identifiers.
- Author `model/blinky.qm` with both graphical state machines, declarations, member/time-event initialization and state actions required for code generation.
- Generate state-machine classes into `generated/`. Keep queue/startup adapters, immutable command-event publishing and transport adapters handwritten where appropriate, but do not hand-maintain state-handler/transition definitions outside the model.
- Add clone-local startup and BSP-facing adapters without depending on source files under `examples/blinky/`.
- Preserve model links/generated headers and licensing notices. The QM tool itself is not redistributed.

### Regeneration/freshness tools

- Provide explicit commands for generation and diagram export using the selected executable.
- Check generator version before processing the model.
- Perform freshness generation in an isolated project-local ignored build directory, retaining the same relative model/output layout; compare generated files without overwriting tracked output during a check.
- Keep normal firmware builds independent of the generator installation.
- Document how to open/edit each state machine in QM and regenerate C++ afterward.

### Build and tests

- Add clone-local CMake targets and top-level `add_subdirectory` integration while preserving the existing example target and sources.
- Reuse the BSP/framework CMake targets and unchanged QP dependency pin; do not duplicate BSP files into the clone.
- Add clone-local host tests for parser/Console and real QP/QV timing behavior using the generated state machines, with the host shim explicitly distinguished from the Arm port.
- Retain sanitizer-enabled testing and test all nine duty-cycle values and one-second steady periods.
- Verify phase preservation, queued-timeout precedence, latest-command-wins, framing/editing, overflow/control recovery, partial-write/backpressure handling, session changes, no-terminal startup and coalesced USB notifications.
- Run the original four host tests without modifying original example files.

### Documentation

- Add `examples/blinky-qm/README.md` explaining layout, model editing, generation, freshness checks, host testing, firmware targets/artifacts and Mac terminal use.
- Update relevant development documentation and task status/index with actual results.
- Record any tool/model incompatibility or required design change and seek approval rather than silently changing the approved strategy.

## Intended artifact locations

- Original firmware: `build/pico2_w/examples/blinky/qpcpp-blinky.{elf,uf2}`.
- Clone firmware: `build/pico2_w/examples/blinky-qm/qpcpp-blinky-qm.{elf,uf2}`.
- Host-test/generated-check temporary outputs: project-local ignored `build/` directories, not external temporary/source locations.
- Exact generated file names and private adapter signatures may be refined within the approved directory/workflow boundaries.

## Acceptance criteria

- Original `examples/blinky/` files remain unchanged.
- Both graphical machines are present in the model and generate the actual compiled state handlers/transitions.
- Generated code is reproducible using QM 5.3.0 with no hand edits, and freshness checking detects stale output.
- The clone builds from tracked generated files without requiring QM during a normal firmware build.
- Both separate firmware images build with the pinned QP/C++ v8.1.5 and existing BSP.
- All clone host tests and the original four host tests pass; test scope/limitations are recorded honestly.
- Both diagrams export and are inspected; model editing/regeneration instructions exist.
- Clone runtime behavior remains identical to the duty-cycle baseline. Report build/host verification separately from actual board validation.
- Any board flashing and Git commit have explicit separate approval.

## Result

Implemented the independent clone under `examples/blinky-qm/`, including the authoritative model, four generated class files, handwritten adapters/helper copies, regeneration/check/diagram tooling, clone-local host tests and README. The top-level build adds the independent `qpcpp-blinky-qm` executable. Updated development documentation links to the clone workflow.

Both generated QActive/QHsm classes are compiled into the clone; all state-handler/transition definitions are generated from `model/blinky.qm`. The generated headers/sources were not hand-edited. The cloned parser/session helpers remain byte-identical to the original baseline but are independent files. Queue storage, immutable duty events and USB wakeup support are handwritten adapters.

Validation performed:

- QM 5.3.0 generation completed with zero warnings/errors. Repeated regeneration/freshness checks produced byte-identical output.
- Exported and visually inspected both SVG/PNG diagrams. Corrected initial-arrow geometry in the model before re-exporting; final diagrams show the initial targets, Blinky hierarchy/timeout transitions and Console internal handlers without overlapping labels.
- Configured `build/blinky-qm-host` with sanitizers and `ENABLE_QM_CHECKS=ON`: all six CTest cases passed (four C++ behavior cases plus two Python/QM generator checks).
- Clone timing tests compile the actual generated classes against the real upstream QP/QV host core and verify all nine duty cycles/steady one-second periods, phase preservation, command precedence/order, reconnect/default behavior and coalesced USB servicing.
- Generator negative tests reject stale, missing and unexpected files and a wrong-version executable. Freshness checking does not overwrite generated sources; test mutations are confined to ignored disposable project-local fixtures.
- Separately configured/built `build/blinky-qm-host-no-qm` with generator checks off and `QM_EXECUTABLE=/nonexistent/qm`: all four behavior tests passed without a generator dependency.
- Configured the normal Pico 2 W Debug build and completed a clean-first build of both separate firmware images with `QM_EXECUTABLE=/nonexistent/qm`. Explicit CMake freshness target also passed with the actual installed QM.
- Both ELF/UF2 artifact pairs exist at the documented original/clone paths. Clone C++ is compiled with `QP_API_VERSION=815`.
- Reran the original four sanitizer-enabled host tests: all passed.
- Verified the clone firmware's Ninja command graph contains neither original-example sources nor a code-generation dependency.
- `git diff --exit-code HEAD -- examples/blinky bsp framework third_party/qpcpp` passed: original application, BSP/framework and dependency pin remain unchanged. QP/C++ is still v8.1.5 at `198b47e7169e27f41d761868fbe83e08d624cd5d`.

Limitations: these are generation, build, host-simulation and diagram checks, not clone board validation. Host tests use a single-threaded critical-section shim and mock BSP; no physical USB enumeration, Arm interrupt behavior or LED timing has been tested for the clone. The shared BSP's existing USB product name is unchanged. No installation/tool upgrade or Git commit occurred. Clone flashing was subsequently separately authorized, as recorded below.

The user subsequently explicitly approved flashing and confirmed BOOTSEL readiness. The agent verified the RP2350 boot volume and copied `build/pico2_w/examples/blinky-qm/qpcpp-blinky-qm.uf2` with `cp -X`. The boot volume disconnected and `/dev/cu.usbmodem142101` enumerated automatically during the 15-second check. This was the clone image, not the original example image.

Status: TASK-0019 completed with generation/freshness checks, both firmware builds, original/clone host tests, diagram inspection, and user-confirmed clone board smoke testing. The user confirmed the console prompt and that `1`, `5`, and `9` produce `Accepted` and the expected 10%/50%/90% on-time over a steady one-second cycle. Timing was visually observed, not instrument-measured. The user also confirmed that `0`, `10`, and the old `1,5` syntax each produce `Invalid` and another prompt while preserving the selected 90% duty cycle. The user confirmed editing `4` to `2` with Backspace/Delete, visible input, `Accepted`, a new prompt, and 20% on / 80% off after the current phase completed. The user confirmed terminal-close/reopen gives a fresh prompt while retaining 20% duty cycle, and a normal USB power cycle restores the default 50%. Host inspection confirmed `/dev/cu.usbmodem142101` is present after the power cycle. The clone's hardware smoke validation passed; exact timing and USB stress behavior were not instrument-measured. After TASK-0020 validation, the user explicitly authorized committing the accumulated implementation, tests and documentation.
