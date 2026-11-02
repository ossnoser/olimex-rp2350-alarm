# PICO-EVB Rev. A diagnostic and four-relay one-shot sequence

For **non-wireless Pico 2 / RP2350-PICO2-HDR on PICO-EVB Rev. A** only.

**Current build can energize REL1..REL4 sequentially after `testall`. It passed automated validation and approved flash/startup verification (READY, four commanded OFF, inputs HIGH); one approved user-entered `testall` produced the expected R1..R4 pulse/gap trace ending STEP4 LOCKED/all-OFF. The user confirmed the physical 1 -> 2 -> 3 -> 4 order, approximately one-second ON phases and one-second all-OFF gaps. This limited unloaded smoke test is not contact/load/current/transient qualification; command flags are not feedback.** Flashing, carrier power and actuation require separate approval. Keep every contact/input terminal and external module disconnected for relay testing. No load/mains capability is certified.

The earlier output-only image passed initial unloaded/input checks in TASK-0022..0024; those are **not hardware validation of this extension**. Its UF2/source snapshot is preserved under ignored `build/task26-baseline/` (`relay-off.uf2`, SHA-256 `c7dabfed7a3621afd4cca2ca96e7813fcb83badcbc562de74f6d41912bba96c6`, and `source.tar`). TASK-0026's REL1-only image passed a user-observed pulse and a repeat request with no activation. Its immediate rollback snapshot is under ignored `build/task27-baseline/` (`relay1-one-shot.uf2`, SHA-256 `6907e91286fee6d96c8f83834a387d4c5ce7245a0d7c0b4ac738eede209950fe`, and `source.tar`). Those observations do not validate this new sequence build.

References: [TASK-0027 sequence](../../docs/tasks/TASK-0027-four-relay-sequence.md), [TASK-0026 implementation](../../docs/tasks/TASK-0026-relay1-one-shot-implementation.md), [coil/power planning](../../docs/hardware/relay-power-and-single-test-plan.md), [Rev. A mapping](../../docs/hardware/pico-evb-rev-a-integration.md).

## One-shot contract

| Command | Behavior |
|---|---|
| `testall` + Enter | While READY in an active terminal session, request the whole fixed R1 -> R2 -> R3 -> R4 sequence below. Then all OFF and LOCKED. |
| `off` + Enter | Force all outputs OFF, cancel pending/running sequence (including OFF gaps), LOCKED even before any activation. |

Nominal sequence: R1 ON 1s -> all OFF 1s -> R2 ON 1s -> all OFF 1s -> R3 ON 1s -> all OFF 1s -> R4 ON 1s -> all OFF/LOCKED. Total seven seconds, with **maximum one output ON**; no extra final gap or repetition.

Old `test1` is now invalid. Commands are exact lowercase, newline-terminated (CR/LF/CRLF), with no echo, duration argument, re-arm command or other relay selection. Invalid/oversized lines do nothing except a bounded/coalesced reply. At most one completed line is consumed per 20 ms poll.

- One complete sequence **per boot**, not one relay per boot or one sequence per terminal connection. A deliberate power cycle/reset is needed to test again; no automatic reset or repeated sequence.
- Requests while pending/PULSING/GAP/LOCKED are rejected. A static pending request is tagged with the USB session generation and revalidated by its owner before actuation. Device-side stale requests/partial frames are discarded; the device cannot know when a host typed a later complete command. Use a trusted manual terminal, close it before flashing, and do not stream or reconnect-automate commands.
- `off` clears outputs as soon as parsed and posts priority cancellation ahead of queued requests. It is not out-of-band/hardware emergency stop: incoming-byte backlog, USB failure or CPU failure can delay/prevent parsing. Remove Pico USB power if behavior is abnormal.
- Detected DTR/session loss, unmount or suspend immediately invokes an opted-in carrier OFF action in the transport callback. Session generation/buffers are invalidated. Diagnostic dispatch cancels/locks an accepted pending/running sequence, including its all-OFF gaps and suspend/resume entirely between polls. Reconnection cannot resume/re-arm a used test.
- Session loss before any request keeps outputs OFF and leaves the unused test READY; simply opening a terminal never activates it.
- No input-to-relay linkage or selected-relay command. Checked BSP activation accepts logical relay 1..4 and clears every other output first.
- Every pulse and OFF gap is 100 framework ticks at 100 Hz: nominal one second, subject to tick quantization/dispatch latency. It is **not a hardware-enforced maximum ON duration** under total CPU/timer/scheduler failure. Contacts must remain empty, operator present.

Do not type `testall` until separate sequence actuation approval and safe preparation are complete. The commands here document software behavior, not authorization to actuate hardware.

## QM and BSP ownership

`model/diagnostic.qm` is authoritative; never hand-edit `generated/`.

- **BlinkyAO**, priority 2, retains the copied `running`/`off`/`on` heartbeat: module onboard LED 500 ms OFF / 500 ms ON, independent of inputs/test. Its retained DUTY path is unused by this interface.
- Blinky owns the generated **RelayTestHsm** (`operational` -> `ready`, `pulsing`, `gap`, `locked`) and separate relay time event. Relay transitions/actions are modeled, not handwritten helper state transitions. Blinky forwards relay events without changing heartbeat phase/timer.
- **DiagnosticAO**, priority 1, owns SAMPLE every 20 ms and coalesced USB_SERVICE. SAMPLE handles raw input sampling and bounded command/report work. USB_SERVICE handles transport and loss detection only, without command parsing/sampling/report formatting.
- `src/` owns queues, static request/session adapters and reporter/parser helpers, with no GPIO numbers/Pico SDK calls. No application source dependency on either existing Blinky example.
- `bsp/carriers/pico_evb_rev_a/` exclusively owns GPIO, monotonic sample time and silicon revision. It exposes checked logical activation 1..4, all-OFF and a four-bit commanded-output mask; that mask is **not feedback**. Invalid relay numbers fail all-OFF.
- Early startup clears all relay latches before enabling outputs, installs fault/session-loss OFF actions, configures inputs, initializes QP/normal BSP/USB, then starts both AOs without waiting for a host.
- Fault OFF is bounded/IRQ-safe and precedes halt, without logging/reset/waits. Optional shared session-loss hooks default disabled for old examples; their identities and standalone behavior remain unchanged.
- Core 0/QV, PRIMASK policy, C++17, QP/C++ v8.1.5, QM 5.3.0, 100 Hz tick and assertions are retained; core 1 is unused. Fixed eight-entry AO queues, static events, no application heap/dynamic event pools.

QM 5.3.0 numeric target paths count transitions as well as states. Adding parent handlers shifts child indices: heartbeat targets are explicitly updated to `off`/`on` (indices 5/6). Source contracts plus actual timing tests guard this; comparing old path strings alone would not prove preserved behavior.

## GPIO and electrical limits

| Resource | GPIO label | Function |
|---|---|---|
| IN1..IN4 | GP2, GP3, GP8, GP9 | Active-low; external 47 kOhm pull-ups, internal pulls disabled |
| REL1..REL4 | GP10..GP13 | Sequential targets; at most one HIGH, all other outputs LOW |
| Module LED | internal GP25 | Independent heartbeat |

GPIO labels are not physical pin positions. EXT1's GPIO23/24/25/29 positions are **GND with standard Pico 2**, not the internal signals; do not use EXT1 GPIO25 for heartbeat. A nonstandard LED-pin override is compile-time rejected for this carrier profile.

The tested module reports revision 2/A2, so E9 applies. Old input testing showed nominal digital functionality on all channels, but only IN1 active GPIO voltage was measured (approximately 0 V at nominal 5 V field excitation). IN2..IN4 analog margins/minimum excitation/noise/tolerance remain unqualified. No automatic pull/resistor workaround is implemented.

User markings support default LEG-5F / 5 V coils on all channels, nominal approximately 72 mA each at 25 degrees C. **Current firmware advertises 250 mA USB power**, not 500 mA. Four nominal coils alone exceed it. One coil at a time, including in this sequence, is only a conditional budget case: actual coil rail beyond L1/current under load remain unmeasured. Source was Mac -> bus-powered Apple USB-C/HDMI/USB-A/USB-C adapter -> Pico USB. Do not infer current capability from the adapter or from the separate 0.5 A field supply, which remains disconnected.

## USB output and bounds

USB product remains `Pico 2 EVB Diagnostic`; VID/PID/board-unique serial and shared CDC layout are unchanged. Product/banner describe configured firmware, not automatic board detection. New builds show a command-help banner; old output-only builds do not.

Raw input snapshots remain:

```text
t=1240ms IN1=HIGH IN2=HIGH IN3=LOW IN4=HIGH
```

Additional timestamped control records show `SEQ=READY/PULSING/GAP/LOCKED`, `STEP=1..4` and four commanded bits, for example `CMD=OFF,ON,OFF,OFF`. STEP is sequence position (the just-completed step during GAP), not electrical feedback. The mask is a **command latch**, never contact feedback. The connection banner's `Relay commands:` line is dynamic. Revision 2 is A2/E9 warning; 3 means A3/A4 (not distinguished); unknown revisions remain unknown.

All lines use CRLF. Timestamps are monotonic sample time, not USB delivery/physical-switching time. Fixed 256-byte in-flight line buffer, one latest pending input/control sample and one latest reply; at most 16 RX bytes/64 TX bytes per poll. Partial lines finish intact; fully unsent updates/replies may be replaced. Control/input reports outrank replies. TX backpressure never gates command execution or OFF. Intermediate states/pulses can be missed/coalesced; this is not an event recorder or hard-real-time guarantee. A fresh session discards partial commands/pending output and prints a new banner/snapshot without re-arming LOCKED.

## Build and regeneration

From repository root (no flashing):

```bash
source env.sh
cmake -S . -B build/pico2 -G Ninja -DPICO_BOARD=pico2 \
  -DPICO_PLATFORM=rp2350-arm-s -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1
cmake --build build/pico2 --target qpcpp-pico-evb-diagnostic
```

Artifact: `build/pico2/examples/pico-evb-diagnostic/qpcpp-pico-evb-diagnostic.uf2`.

Normal Pico 2 builds produce three separate images; Pico 2 W retains only the two existing examples. Normal builds use stored generated C++ without running QM. Physical board variant must be confirmed before a later flash; the RP2350 boot-volume label does not distinguish wireless boards.

```bash
python3 examples/pico-evb-diagnostic/tools/qm_codegen.py regenerate
python3 examples/pico-evb-diagnostic/tools/qm_codegen.py check
python3 examples/pico-evb-diagnostic/tools/qm_codegen.py diagrams
```

Open `model/diagnostic.qm` in QM 5.3.0. Use `--qm PATH`/`QM_EXECUTABLE` if needed. Checks run in project-local sandboxes, never overwrite tracked output, and reject another generator version/stale/missing/extra files. Six generated C++ files cover three machines; SVG/PNG diagrams go to `build/pico-evb-diagnostic-diagrams/`. Explicit CMake targets: `pico-evb-diagnostic-regenerate`, `pico-evb-diagnostic-check`. No automatic tool installation.

## Automated validation

```bash
cmake -S examples/pico-evb-diagnostic/tests -B build/pico-evb-diagnostic-host \
  -G Ninja -DENABLE_QM_CHECKS=ON
cmake --build build/pico-evb-diagnostic-host
ctest --test-dir build/pico-evb-diagnostic-host --output-on-failure
python3 tests/bsp/verify_builds.py
python3 examples/pico-evb-diagnostic/tests/verify_builds.py
```

37 diagnostic cases with QM enabled (35 without), plus the existing 13 regression cases, currently pass. Clang/GNU native address/undefined-behavior sanitizers default ON. Coverage includes real QP/QV generated timing/lockout/cancellation, flood/backpressure, stale/pending requests, before/after-use reconnect, between-poll suspend/resume, exact parser framing, actual opted-in transport callbacks, fault hooks/command latch, exact four-relay ordering/100-tick ON and OFF phases, aborts in every pulse/gap, priority cancellation of a queued gap timeout, four-pin one-bit GPIO writes/invalid-index rejection, generation checks and heartbeat/SDK isolation. Existing four images and new diagnostic build/link identity isolation pass.

Host-only exceptions escape idle/halt for tests; production adds no exception path. Mocks/build checks do not validate physical USB, coil voltage/current, actual contact timing or Arm IRQ/transient behavior. The current sequence build has startup/USB and one manual sequence trace, with user-confirmed physical order and approximate ON/OFF pattern; the older REL1-only observations are not transferable sequence/contact/fault/transient validation.

## Later hardware test gates

Keep the board unpowered/unwired until separate preparation/flash/test approval. After that, the intended minimal sequence is:

1. Confirm non-wireless module, Rev. A, all four default 5 V LEG-5F bodies, empty contacts/field terminals and disconnected external sources/modules. Program/verify the correct image; preserve old firmware for rollback. No actuation command during enumeration/banner checks.
2. Pico USB only: verify heartbeat, READY, commands all OFF and unloaded inputs HIGH. Close/reopen before any test may leave READY; after any accepted sequence/OFF, reconnect must show LOCKED.
3. Verify the actual coil rail beyond L1 using safely identified/accessible points and DC-voltage meter setup. PWR1 is a mux input, not a substitute rail point. Use insulated leads installed unpowered; if access is unsafe, stop. The prior 4.9 V VSYS reading was unloaded and is not this measurement.
4. Only after explicit sequence actuation approval, keep an operator present and issue `testall` once. Observe REL1..REL4 in order, each about one second ON with one second all-OFF between relays; never two indicators ON, then all OFF/LOCKED after REL4. Capture software state output without interpreting it as contact feedback. Disconnect power on unexpected behavior; no automatic retries.
5. Loaded-voltage/current/transient observations depend on suitable instrumentation. A slow meter may miss a one-second dip; do not silently lengthen the pulse or change power sources. STOP, session loss, restart and repeat tests each need reviewed sequencing; all loads remain disconnected.
6. End unpowered and unwired. No simultaneous coils, repeat sequence/reset/re-arm, changed durations or input-driven control is authorized by this implementation milestone.

A frozen CPU may leave the coil ON until sole USB power is removed. A de-energized relay's NC contact is still closed. No software timeout, dark LED or empty-load bench test establishes a safety interlock, contact rating or permission for mains/load wiring.
