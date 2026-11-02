# TASK-0022 - PICO-EVB Rev. A diagnostic firmware plan

## Status

Approved / completed for the initial relay-off diagnostic milestone. Extended electrical/fault/load validation remains outside the completed smoke-test scope.

Follow-up: TASK-0026 extends this example with a manual REL1 one-shot interface. The initial output-only source/UF2 is archived under ignored `build/task26-baseline/`. Hardware results below belong to the original output-only image, not the newer command-capable build; consult the current example README before using it.

The user approved recording the plan and subsequently explicitly approved implementing TASK-0022. Firmware/build changes and automated tests within this scope are authorized. Flashing, carrier power-on/testing and commits remain separately gated.

## Objective

Prepare the first carrier-aware firmware milestone for a non-wireless Pico 2 / RP2350-PICO2-HDR on PICO-EVB Rev. A: command every relay OFF, run the familiar onboard-LED heartbeat, and report silicon revision and raw inputs over USB. Preserve both existing Blinky examples for regression/reference use.

## Research and hardware context

[TASK-0021](TASK-0021-pico-evb-rev-a-investigation.md) and [the Rev. A investigation](../hardware/pico-evb-rev-a-integration.md) establish the source-derived routing, power arrangement and limitations:

- Relay drivers: GP10..GP13, active-high; LOW commands OFF.
- Inputs: GP2/3/8/9, active-low, external 47 kOhm pull-ups to 3.3 V.
- Pico USB is the proposed sole supply/data connection. Carrier USB-C and power terminal remain disconnected. No carrier power-selection jumper change.
- All relay contacts, input terminals and external modules remain disconnected for the initial test. No mains, controlled loads, input excitation or RS485 wiring.
- The user has a multimeter for later static supply/input-level checks. It cannot establish absence of brief startup glitches.
- EXT1's GPIO23/24/25/29 labels correspond to GND positions with standard Pico 2, not the module's internal GPIO signals. The internal GP25 onboard LED remains available.
- Silicon revision, actual assembly/orientation, fitted relay part/current budget and physical voltage margins are not established by schematic research.

Earlier setup/research decisions 1-5 are recorded in TASK-0021. The diagnostic decisions below continue the user-approved sequence.

## Individually approved diagnostic decisions

6. **Separate milestone:** create `examples/pico-evb-diagnostic/`, leaving both existing examples unchanged. First command all four relays OFF early; report silicon revision/raw inputs. No relay activation commands or automatic input-to-relay actions.
7. **Modeling:** QM 5.3.0-generated QActive/QHsm code, model as source of truth, generated C++ retained for normal builds, explicit regeneration/freshness checks. Keep QP/C++ v8.1.5 pinned.
8. **Initial hardware scope:** non-wireless `pico2` plus PICO-EVB Rev. A only. Pico 2 W configurations retain their existing two examples and do not include the diagnostic target.
9. **BSP isolation:** add `bsp/carriers/pico_evb_rev_a/`, linked only by the diagnostic. It owns carrier GPIO mapping, early relay-off initialization and input access. Application/model code has no GPIO numbers or Pico SDK calls. Existing Blinky targets do not initialize carrier pins.
10. **Fault policy:** on a detected framework/initialization error, force relays OFF, then halt without automatic reset or USB logging inside the fault handler. Preserve existing examples' fault behavior. This does not guarantee control during boot ROM execution, electrical faults or total CPU failure.
11. **Inputs:** use the external pull-ups and disable Pico internal pulls. Report HIGH/LOW raw values, where HIGH is inactive for disconnected working channels and LOW is an active optocoupler. No alarm interpretation, debounce or relay actions. Report the A2/E9 caveat without automatically altering pulls/hardware.
12. **Cadence:** sample every 20 ms. Send a full snapshot on terminal connection, then snapshots when sampled inputs change. No unchanged-status stream. Pulses shorter than sampling intervals may be missed; this is not an event recorder.
13. **Format:** connection banner identifies firmware/carrier, raw silicon revision interpreted as A2 or A3/A4 when known, A2/E9 warning when applicable, and relay commands OFF explicitly distinguished from actual contact feedback. Snapshots have sample-time timestamps, for example `t=1240ms IN1=HIGH IN2=HIGH IN3=LOW IN4=HIGH`. Use CRLF.
14. **Backpressure:** fixed-memory, nonblocking output. Continue sampling; coalesce pending snapshots to the latest value. Complete a partially transmitted line before switching to newer data. Clear pending output on disconnect; reconnect with fresh banner/snapshot. No unbounded history or guarantee of every transition being delivered.
15. **Output-only interface:** no command parser/device echo. Discard typed characters in bounded batches. No USB input can energize a relay.
16. **Heartbeat:** use the Pico's onboard LED at 500 ms on / 500 ms off, independent of input levels or terminal connection. Do not blink relays or their indicators, and do not use EVB EXT1's misleading GPIO25 position.
17. **Independent HSM copy:** the user explicitly rejected directly linking/reusing the existing Blinky sources. Copy the Blinky HSM into the diagnostic's own QM model so it can later be extended for input/relay behavior. Preserve the existing examples. Such future behavior is not authorized now.
18. **Two active objects:** copied Blinky AO priority 2 for heartbeat; Diagnostic AO priority 1 for input sampling and USB reporting. Future input-event routing to an extended Blinky HSM remains a later design task.
19. **Resources:** eight-entry event queue per AO, fixed 256-byte transmit buffer, at most 64 transmitted bytes and 16 discarded received bytes per 20 ms poll. No application heap allocation or dynamic event pools. Sampling continues under backpressure.
20. **Startup order:** after SDK runtime reaches `main()`: initialize relay outputs LOW; install the diagnostic relay-off fault action; configure inputs; initialize QP and the ordinary Pico BSP; start both AOs/run without waiting for a terminal. This does not control boot-ROM behavior.
21. **USB identity:** only this firmware advertises `Pico 2 EVB Diagnostic`. Existing examples' names, CDC configuration, VID/PID and board-unique serial identity remain unchanged.
22. **Layout:** example-local `model/diagnostic.qm`, `generated/`, `src/`, `tools/`, `tests/`, `CMakeLists.txt`, and `README.md`. Both machines live in this model; there is no source dependency on either existing example. Carrier GPIO implementation remains in the dedicated BSP directory.
23. **Build target:** `qpcpp-pico-evb-diagnostic`, available only for `pico2`, alongside—not replacing—the existing two examples. Normal builds consume stored generated C++ without requiring QM. Expected artifact: `build/pico2/examples/pico-evb-diagnostic/qpcpp-pico-evb-diagnostic.uf2`.
24. **Diagnostic HSM:** initial -> one `active` state. Internal SAMPLE event performs 20 ms input/report work; internal USB_SERVICE event services USB without sampling/formatting. Connection tracking and buffering reside in a testable handwritten helper. Copied Blinky HSM independently manages heartbeat.
25. **Validation:** host safety/init/fault tests; reporter revision/input/time/partial-output/coalescing/reconnect tests; generated-HSM/freshness tests including unchanged heartbeat; existing 13 checks and unaffected existing targets; later separately approved unloaded hardware/multimeter tests. Static meter checks do not prove glitch-free startup.
26. **Planning gate:** document this specification and task index only, then obtain separate implementation approval. No code, flashing, power-on or commit authorized by this gate.

## Model and application behavior

The model contains an independent copy of the established Blinky `running`/`off`/`on` HSM, including its existing phase/timer semantics. It starts at 50%; the diagnostic exposes no duty-cycle command path and does not send duty updates or input events to it in this milestone. Retaining the copied handler for later evolution does not authorize new heartbeat controls or any relay-ON behavior.

The new Diagnostic HSM owns its periodic sample event and USB-service dispatch. Keep USB notification coalescing and prompt transport servicing in the established pattern: interrupts notify the AO, and stack work executes in application context. USB-service events must not bypass the 20 ms sample/report budget.

Silicon revision output must not falsely distinguish A3 from A4 solely from the shared revision value. Show the raw value, interpret known values accurately, and label unknown values as unknown rather than guessing. The field values describe configured carrier inputs, not automatic detection of a physically fitted carrier. Standalone programming without the EVB can leave input pins floating.

Report commanded relay states, never pretend the board has contact/coil feedback. The initial firmware never energizes a relay. Existing standalone firmware is not a substitute for this carrier initialization.

## Approved implementation scope

### Carrier BSP and shared extension points

- Implement private carrier mapping for GP10..GP13 and GP2/3/8/9 under `bsp/carriers/pico_evb_rev_a/`.
- Preload LOW before enabling relay outputs; provide a bounded, IRQ-safe force-OFF path requiring no QP/USB/formatting/waiting operations.
- Expose input sampling, monotonic sample timestamp and silicon revision through SDK-independent carrier interfaces.
- Configure external-pull-up inputs with internal pulls disabled; do not add automatic electrical workarounds for A2.
- Add narrowly scoped shared-BSP extension points where necessary for an optional fault action and startup-selected USB product name. Existing users must retain default behavior/identity when no diagnostic option is installed. Keep existing public call signatures backward compatible.
- Link carrier implementation only into the diagnostic target; verify existing firmware has no carrier initialization dependency or relay-pin side effects.
- Shared changes are limited to supporting the approved fault/identity behavior; do not refactor unrelated drivers or upgrade frameworks/tools.

### Example and generation workflow

- Copy the heartbeat HSM/model content and required supporting data types into the independent example; do not link application sources from existing examples.
- Author DiagnosticAO in the same model and generate the actual classes/handlers. Do not hand-maintain generated state transitions.
- Keep startup, transport adapters, reporter formatting/coalescing helpers and queue storage in `src/`, free of GPIO numbers/SDK headers.
- Use QM 5.3.0 via explicit generator commands with executable override and version check, matching the established tracked-code/freshness approach. Do not require standalone QMC or install tools automatically.
- Perform freshness work in project-local ignored build directories; never overwrite tracked source during a check.
- Keep new code/model/generated files under GPL-3.0-or-later and preserve applicable notices.

### Build and documentation

- Add the Pico 2-only target and carrier library without removing the existing four board/example build combinations.
- Keep C++17, QV/core 0, PRIMASK policy, 100 Hz framework tick, assertions and current version pins. Core 1 remains unused.
- Document source ownership, QM editing/regeneration, build/test commands, output format/coalescing limitations and the exact new artifact path.
- Document the unpowered assembly and separately approved first-power checklist, physical pin/rail measurement points and interpretation limits before any test is authorized.
- Preserve the existing uncommitted research documentation from TASK-0021.

## Automated validation plan

1. **Carrier SDK-stub tests:** initialization sequence, LOW-before-output, correct masks/pins, disabled input pulls, unchanged unrelated GPIO, raw input packing/time/revision, and no HIGH relay writes. Seeded HIGH latch values are allowed only in host mocks to verify force-OFF behavior, never as a physical test.
2. **Fault/identity extension tests:** diagnostic callback invoked before halt path, no formatting/waiting in that path, and correct selected product identity. Verify absent hooks leave existing BSP behavior/identities unchanged. Do not add a console fault-injection command.
3. **Reporter tests:** known/unknown silicon values and E9 warning, initial full snapshot, changes only, sample timestamps, bounded RX discard/TX attempts, partial lines, latest-pending coalescing under backpressure, fresh reconnection without stale data, and output-only behavior.
4. **Real QP/QV host tests:** generated classes, 20 ms sample schedule, independent 500 ms heartbeat, no heartbeat change from inputs/typed USB data, coalesced USB wakeups without off-schedule sampling, fixed queues and no relay activation.
5. **Generation checks:** byte-identical fresh output, rejected stale/missing/extra output/wrong generator version, and exported/inspected diagrams for both machines. Verify the initial copied Blinky state structure/actions retain the established behavior.
6. **Regression/build checks:** existing 13 CTest cases and original four-image matrix; new diagnostic builds only for `pico2`; existing examples/models are unchanged. Verify only the new target links carrier code and advertises its special USB name. No actual flash or power-on is part of automated validation.

## Gated hardware validation plan (not yet authorized)

Before power-on:

- Confirm physical non-wireless Pico 2 and Rev. A carrier, orientation/alignment, unloaded terminals, disconnected modules and no EVB external power sources.
- Obtain explicit flash authorization; program/verify the Pico standalone where practicable. Interpret floating standalone input readings appropriately.
- Review the measurement procedure with the user: meter in high-impedance DC voltage mode, correct ground/reference points and precautions against adjacent-pin shorts. Do not use resistance/continuity/current mode across powered rails.
- Assemble unpowered, then obtain separate carrier-power/test approval.

Initial powered checks use only Pico USB. Record 3.3 V and VSYS/carrier-rail measurements, USB identity/banner, silicon revision, heartbeat, all relay indicators commanded off and four unloaded raw inputs expected HIGH. The report must distinguish commands/indicator observations from actual contact feedback or a safety certification. Stop/disconnect on unexpected energization, rail behavior or heating.

Check terminal reconnect and normal reset/power-cycle behavior while keeping all contacts/field terminals unwired. Do not claim a multimeter excludes short transients. No relay activation, input excitation, RS485 attachment or mains test is permitted in this milestone; those need separate designs/approvals.

## Acceptance criteria

- New target follows every approved safety/output/sampling/heartbeat policy, with two independently modeled AOs and no source dependency on existing examples.
- Both existing Blinky examples/models and their hardware behavior remain unchanged; any shared BSP additions have backward-compatible defaults.
- No application/graphical-model GPIO numbers or direct SDK calls; no relay-ON path in the diagnostic interface.
- Generated code is reproducible and compiled into firmware; normal builds do not require QM.
- New tests and existing regressions pass, including target/link isolation and product identity.
- Build/host observations and separately authorized hardware observations are clearly distinguished, with unperformed cases left pending.
- Documentation includes current research caveats and does not certify electrical/load safety.
- No unauthorized source changes outside the project, flashing, powering or commit.

## Result

Implemented the approved independent example, carrier BSP and optional backward-compatible shared-BSP fault/identity hooks. The relay initializer first clears all four SIO latches together, then configures each relay pin with LOW preloaded before output enable. A compile-time guard rejects nonstandard LED-pin overrides for this carrier profile to prevent heartbeat from driving a relay/input pin.

Validation results:

| Validation | Result |
|---|---|
| Existing handwritten-example CTest cases | 4/4 passed |
| Existing QM-example CTest cases, generator checks enabled | 6/6 passed |
| Existing BSP LED-adapter CTest cases | 3/3 passed |
| New diagnostic CTest cases, generator checks enabled | 7/7 passed |
| Existing four firmware build combinations | Built and inspected successfully |
| New Pico 2 diagnostic firmware | ELF/UF2 built successfully |
| Carrier build/source/symbol/USB identity isolation | Passed for all five images; diagnostic target absent from Pico 2 W |
| New QM-generated output and negative freshness fixtures | Passed using installed QM 5.3.0 |
| Initial copied Blinky class versus existing model | Exact XML class copy verified, including actions/timing/diagram |
| Both exported QM diagrams | Rendered and visually inspected |
| Existing examples/models and QP submodule | No source changes (`git diff --exit-code HEAD -- examples/blinky examples/blinky-qm third_party/qpcpp`) |
| Whitespace checks | `git diff --check` passed |

Total: **20 CTest cases passed**, plus build-graph/image inspections. Native tests use address/undefined-behavior sanitizers; the host QP/QV port and SDK stubs do not validate Arm interrupt behavior or physical wiring. The fault test compiles the actual shared `Q_onError`/timer-start hook, with a host-only throwing callback to escape halt after verifying interrupts are masked and relay bits cleared. No exception path was added to production.

Commands and test/measurement limitations are in `examples/pico-evb-diagnostic/README.md`. Build inspection scripts: existing `tests/bsp/verify_builds.py` and new `examples/pico-evb-diagnostic/tests/verify_builds.py`. Build logs/diagrams/codegen fixtures remain under ignored `build/`. Existing TinyUSB configuration macro-redefinition warnings remain; no unrelated USB configuration refactor was performed.

New artifact: `build/pico2/examples/pico-evb-diagnostic/qpcpp-pico-evb-diagnostic.uf2`.

- SHA-256 at validation: `c7dabfed7a3621afd4cca2ca96e7813fcb83badcbc562de74f6d41912bba96c6` (recheck if rebuilt).
- `arm-none-eabi-size`: text 119112, data 0, bss 4076 bytes. These are reported sections, not a complete runtime stack/heap budget.

The independently copied HSM is ready for future approved extensions, but this milestone has no input-to-relay behavior or relay-ON path. Output reports command state, not contact feedback. The multimeter checklist remains separately gated and does not certify glitch-free startup or load safety.

At implementation handoff, flashing, physical carrier assembly/power/testing, electrical measurements, post-change hardware regression and commit/push had not been performed. TASK-0021 research is preserved.

## Standalone flash authorization and validation

The user confirmed the non-wireless Pico 2 was separate from the EVB and unpowered, explicitly approved flashing this diagnostic to the standalone module, entered BOOTSEL following instructions, and then said to proceed. This permits Pico USB power/programming only; carrier assembly/power/testing and commits remain separately gated.

Pre-copy checks passed: `/Volumes/RP2350/INFO_UF2.TXT` reports UF2 Bootloader v1.0, Model Raspberry Pi RP2350, Board-ID RP2350. This generic ID does not distinguish wireless variants; physical identification relies on the user-confirmed non-wireless module. The selected 230912-byte UF2 matches the SHA-256 recorded above. Copied the exact verified image with `COPYFILE_DISABLE=1 cp -X ... /Volumes/RP2350/`. The copy succeeded; after reboot the boot volume disappeared and `/dev/cu.usbmodem142101` appeared. Read-only macOS USB registry inspection confirms product `Pico 2 EVB Diagnostic`.

Standalone flashing and USB enumeration/product identity are verified. The user subsequently confirmed that the onboard LED blinks roughly 0.5 seconds on / 0.5 seconds off. This is a visual cadence observation, not an instrumented timing measurement.

The user separately approved briefly opening USB serial to read the banner/revision without sending commands. Opened `/dev/cu.usbmodem142101` nonblocking at 115200, asserted DTR, read for three seconds, then dropped DTR, restored terminal settings and closed the descriptor. No application bytes were sent. The 291-byte capture is in ignored `build/pico-evb-diagnostic-standalone-serial.txt`.

Observed banner:

```text
Pico 2 EVB Diagnostic / configured PICO-EVB Rev. A
RP2350 raw revision=2 (A2)
WARNING: GPIO E9 applies; validate input voltage margins.
Relay commands: OFF OFF OFF OFF (no contact feedback)
Inputs: HIGH=inactive, LOW=active; raw/unfiltered
t=150706ms IN1=LOW IN2=LOW IN3=LOW IN4=LOW
```

The tested module is therefore A2 according to the SDK revision API; E9 applies. This is a per-module observation, not a claim about all Pico 2 units. All four LOW readings were recorded with the module standalone, without the carrier's 47 kOhm pull-ups. They do not establish active field inputs or validate any voltage threshold/level. Actual GPIO voltages have not been measured, and no automatic pull/resistor changes were made.

Standalone flash, product enumeration, user-observed heartbeat and initial banner/revision reporting have passed. The user separately approved a brief serial reopen check. Reopened the same port with DTR, captured for three seconds, then dropped DTR/restored terminal settings/closed it without sending application bytes. The 291-byte capture is in ignored `build/pico-evb-diagnostic-standalone-reconnect.txt`. Exactly one fresh banner and snapshot were received; sample uptime advanced from 150706 ms to 271526 ms, consistent with no reboot during the session change. The revision remained 2/A2 and standalone raw inputs remained LOW. Explicit terminal reopen reporting passed; heartbeat timing during this check was not instrumented.

Physical carrier behavior remains pending. No relay was intentionally energized and the EVB remains disconnected/unpowered. Further serial checks, assembly and carrier power/test steps remain separately gated. Unplug Pico USB before physical assembly inspection; do not mount/remove the module while powered.

## Carrier assembly checkpoint

The user declined photographic/orientation inspection, stated familiarity with insertion, and asked whether to mount the module. Instructions were to mount only with all power disconnected, leave relay/input terminals and UEXT modules disconnected, and keep EVB USB-C, external power terminals and Pico USB unplugged. The user then reported the Pico mounted. Assembly is user-reported, not visually/continuity-verified by the agent.

The agent requested approval for the first unloaded carrier power-up using only Pico USB, keeping all other sources/terminals/modules disconnected and stopping on unexpected activation/heating. The user then reported that the board was powered with Pico USB and that everything seemed as expected. This records a user-performed first power-up, not an agent-observed/instrumented test. Supply voltages, mounted-board raw input readings, reset behavior and contact state have not yet been checked. Proceed with one static voltage check at a time; do not add field excitation or relay actuation.

Static measurement 1: after instructions to use DC voltage mode, V input socket, verified system GND and Pico `3V3(OUT)`, the user reported **3.3 V**. This is consistent with the nominal logic supply. Static measurement 2: at the requested Pico `VSYS` point relative to the same system GND, the user reported **4.9 V**, consistent with USB-derived VSYS. The carrier coil rail beyond L1, GPIO levels and transient behavior remain unmeasured.

The user then explicitly approved a brief mounted-board USB read. Opened the same port for three seconds, sent no application bytes, dropped DTR/restored settings/closed afterwards. Capture: ignored `build/pico-evb-diagnostic-mounted-serial.txt` (295 bytes). The expected banner again reported raw revision 2/A2 and commanded relays OFF with no contact feedback. One snapshot was received:

```text
t=614340ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
```

No changed-state report was received during the capture window. This matches unloaded inputs with carrier pull-ups present, unlike the standalone LOW readings. It does not establish analog voltage margins, active-input behavior, every short pulse, coil/contact state or glitch-free startup. Raw mounted-board inactive-level reporting passed.

Static input measurement: the user initially reported 0 V on all four requested GPIO points. Because this contradicted the digital HIGH snapshot, the agent instructed unplugging Pico USB and asked whether the readings were from GPIO/header points or isolated field screw terminals. The user explicitly corrected the reading to **approximately 3.27 V on all four pins**. The initial zero values are retracted, not treated as a confirmed hardware fault or actual GPIO LOW measurement. Exact cause of the initial mistake was not established.

The corrected user-reported GP2/GP3/GP8/GP9 measurements agree with inactive HIGH and the 3.3 V supply; they do not validate optocoupler sink/current margins at active input levels. No pulls/resistors or firmware settings were changed. Relay-control measurement follow-up: after the requested GP10/GP11/GP12/GP13 (Pico physical 14/15/16/17) checks, the user reported **0 V, 0 V, 3.3 V, 3.3 V**. These values do not match the intended LOW levels if measured on the actual relay GPIOs. Instruct disconnecting Pico USB while clarifying probe points before continuing.

A possible explanation, not a confirmed diagnosis, is GPIO-versus-physical-pin numbering confusion: GP14/GP15 are not relay controls, while GP16/GP17 have EVB I2C pull-ups and can read near 3.3 V. Obtain confirmation of the actual GPIO labels probed; do not interpret the reported values as either proven relay activation or a passed relay-voltage test. No firmware/hardware changes or additional power/test steps are authorized to address this discrepancy yet.

The user confirmed GPIO-versus-physical-position confusion in the preceding relay measurement, then reported a new reading of **3.3 V at each GPIO label 10/11/12/13**. This still contradicts the intended LOW relay outputs and is not a passed test. Actual probe/reference locations and relay activation state remain unverified. Do not assume the firmware or hardware is correct from the OFF banner, or infer contact state from these reports.

Stop the powered test: instruct unplugging Pico USB and do not request another powered measurement until probe/reference points are clarified. Request an unpowered photograph showing the exact system-ground and GPIO10 points used. No code, pull, wiring or supply changes have been made to address the discrepancy.

The user subsequently reported **0 V on GPIO10/11/12/13**, matching the intended outputs. The agent explicitly clarified that this must be measured with Pico USB connected and the heartbeat running, not with the board unpowered. The user affirmed that the outputs were measured correctly and instructed continuing the tests. Record the corrected LOW values as user-reported verification under the stated powered measurement conditions; probe locations/power were not independently observed and no photograph was supplied. Earlier conflicting values are superseded, not treated as proof of hardware activation or a diagnosed firmware fault. No firmware or electrical changes were made.

Next authorized continuation: normal Pico-USB-only power-cycle observation with all contacts/input terminals/modules still disconnected, followed by USB identity/banner/raw-input checks. No relay actuation, field excitation, BOOTSEL test or supply changes. The coil rail beyond L1 remains unmeasured and must be verified at a safely accessible point before any future relay-energization milestone. Multimeter/LED observations cannot prove absence of brief transients or contact state.

## Mounted-board normal power-cycle check

Following the continuation request, instructions were to unplug Pico USB for five seconds, reconnect only Pico USB with all terminals/modules still disconnected, and observe heartbeat plus all four relay indicators. The user confirmed the heartbeat resumed and relay indicators stayed OFF. This is a visual observation, not oscilloscope/contact feedback.

The agent then attempted the planned read-only post-cycle USB check. macOS registry inspection confirmed `Pico 2 EVB Diagnostic`, but opening `/dev/cu.usbmodem142101` failed with `EBUSY`. Read-only `lsof` showed the user's `screen` process (PID 54606) holding the device. No serial descriptor was opened by the agent, no DTR/application bytes were sent, no process was killed, and no post-cycle serial capture was created. The user then pasted the post-cycle terminal banner, confirming the configured Rev. A diagnostic identity, raw revision 2/A2 with E9 warning, relay commands OFF (no contact feedback), and this snapshot:

```text
t=23700ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
```

This is user-provided terminal evidence, not an agent-created serial capture. Combined with the user's heartbeat/relay-indicator observation and macOS USB identity check, the normal Pico-USB-only power-cycle smoke check passed. The agent did not close or interrupt the user's `screen` session.

## Initial milestone outcome and remaining limits

The authorized initial relay-off diagnostic milestone is complete: independent QM model/carrier BSP implemented, 20 automated cases and all five firmware builds/inspection checks passed; standalone flash/identity/banner/reopen and unloaded carrier static/input/power-cycle smoke checks passed within the stated observations.

Recorded user measurements: 3V3(OUT) 3.3 V; VSYS 4.9 V; unloaded GP2/GP3/GP8/GP9 approximately 3.27 V each; corrected relay-control GP10..GP13 readings 0 V each under the requested powered conditions. Prior pin/reference-confused values are superseded and their clarification remains documented above. This is not an independent instrumented audit of probe placement or contact state.

Not tested: field input excitation/active LOW margins (particularly A2/E9), coil rail beyond L1, relay actuation/contact/load behavior, BOOTSEL/watchdog/debugger/fault power transients, long-term USB stress or safety-critical performance. Static readings/LED observations do not exclude brief glitches. These remain separately scoped/gated; initial completion does not certify electrical safety or the final alarm-node design. No commit/push or later firmware/hardware changes are authorized by this completion.

The next proposed investigation is low-voltage input activation using the existing raw-input diagnostic, with relays held OFF. First establish available excitation equipment, then obtain a separate documented task/test approval before any field-terminal wiring or input excitation.
