# PICO-EVB Rev. A diagnostic and individual relay commands

For **non-wireless Pico 2 / RP2350-PICO2-HDR on PICO-EVB Rev. A** only.

**TASK-0028 build: a relay stays ON until explicitly switched OFF, detected session loss/suspend, fault OFF or power removal. There is no automatic timeout.** This image passed automated validation and an approved flash/startup check (IDLE, four commanded OFF, inputs HIGH). No application command was sent during startup. A subsequent approved user-entered REL1 ON/selected-OFF test produced the expected IDLE -> ON -> IDLE trace; the user confirmed REL1 stayed ON until selected OFF and the others stayed OFF. The user subsequently reported all-four manual relay tests successful, no second simultaneous activation, and global `off` working for any active relay. No further trace/pair matrix or explicit wrong-channel selected-OFF observation was supplied. Unpowered/unwired cleanup was user-confirmed; physical loss/fault paths and contact/current/thermal behavior are not qualified. Flashing, powered testing and commits require separate approval. All contact/input terminals, external modules/field supplies and carrier USB-C must remain disconnected for relay tests; Pico USB only. No load/mains capability is certified.

[TASK-0028](../../docs/tasks/TASK-0028-individual-relay-commands.md) is the approved current contract. The prior [TASK-0027](../../docs/tasks/TASK-0027-four-relay-sequence.md) sequence image passed an approved user-observed unloaded R1..R4 sequence and cleanup; that does **not** validate latched operation, loaded rail/current, thermal margins, physical cancellation or actual contacts in this new image. Immediate rollback: ignored `build/task28-baseline/source.tar` and `four-relay-sequence.uf2`, SHA-256 `65010399c650ebc91a167196b605aeb7a21085d47c5e1cdd07ae8d91411e1856`; committed baseline `1a70535`. Earlier REL1-only/relay-OFF snapshots remain under `build/task27-baseline/` / `build/task26-baseline/`.

References: [coil/power research](../../docs/hardware/relay-power-and-single-test-plan.md), [Rev. A mapping](../../docs/hardware/pico-evb-rev-a-integration.md).

## Command contract

| Command | Behavior |
|---|---|
| `on N` + Enter | N=1..4. If IDLE, all commands OFF, no pending ON and terminal session active: request only N ON, with no timeout. |
| `off N` + Enter | Clear only N immediately when parsed; cancel a matching pending ON. Another active/pending relay is unaffected. Already OFF is harmless. |
| `off` + Enter | Clear all outputs and cancel all pending ON requests, with priority over queued activation. |

Commands are exact lowercase ASCII, single spaces and a single relay digit, newline-terminated (CR/LF/CRLF), with no echo. `testall`, `test1`, `on 0`, `on 01`, trailing spaces/arguments and other malformed/oversized lines are invalid and do not change outputs. At most one completed command is consumed per 20 ms poll.

- **Maximum one commanded ON.** Every ON while another ON is pending or any relay is ON (including that same relay) is rejected, not queued/switched/toggled. It does not disturb the active relay.
- OFF/cancellation returns the controller to reusable IDLE. Fresh ON commands work without reset/power cycle; no lockout, sequence, gap, pulse or relay timer remains.
- Opening a terminal or changing an input never activates a relay. No input-to-relay linkage.
- A pending ON is tagged with channel and USB generation and revalidated by BlinkyAO. Cancelled/stale events cannot consume another channel's pending request or turn an unrelated active relay OFF.
- Detected DTR/session loss, unmount or suspend immediately invokes the opted-in carrier all-OFF callback, invalidates transport buffers/generation, cancels pending activity and returns IDLE. Reconnection never resumes/reactivates: it requires a fresh explicit ON command in the new active session.
- The terminal must deassert DTR on close for closure to be detectable. Closing a UI while its process keeps the port/DTR open is not observable as session loss.
- **Communication loss means detected transport/session events, not an inactivity watchdog.** An open but silent/stalled host, a frozen CPU or undetected transport failure can leave the coil ON indefinitely. OFF is parsed software control, not a physical emergency stop; byte backlog can delay it. Remain present and remove sole Pico USB power on unexpected behavior. Do not leave the board energized unattended.
- A device cannot authenticate when the host originally typed a later complete command. Busy rejection is at parsing/owner validation, not host typing time. Use a trusted manual terminal, close it before flashing and do not stream or reconnect-automate commands. Host-buffered bytes arriving later are not a device-side queued ON guarantee.

The examples above document software behavior, **not permission to energize hardware**. No automatic OFF must be assumed after `on N`.

## QM and BSP ownership

`model/diagnostic.qm` is authoritative; never hand-edit `generated/`.

- **BlinkyAO**, priority 2, retains the independent copied `running`/`off`/`on` heartbeat: onboard LED 500 ms OFF / 500 ms ON. Its retained DUTY path is unused by this interface.
- Blinky owns generated **RelayTestHsm** (`operational` -> `idle`, `on`), which tracks active logical relay 0..4. Its transitions, matching-channel OFF guard, global/session OFF and activation actions live in the model. There is **no relay time event**. Only heartbeat and input-sample timers remain.
- **DiagnosticAO**, priority 1, owns SAMPLE every 20 ms and coalesced USB_SERVICE. SAMPLE handles raw inputs and bounded commands/reports; USB_SERVICE handles transport/loss only, without command parsing/sampling/formatting.
- `src/` owns fixed queues, immutable static channel events, a single pending ON record and reporter/parser helpers; no GPIO numbers/SDK calls or source dependency on either existing Blinky example.
- `bsp/carriers/pico_evb_rev_a/` exclusively owns GPIO, monotonic sample time and silicon revision. Checked activation 1..4 clears every output before enabling one; invalid activation fails all-OFF. Checked selected deactivation clears only its pin/command bit; invalid deactivation makes no changes. The four-bit latch is **command tracking, not feedback**.
- Early startup clears relay latches before enabling outputs, installs fault/session OFF hooks, initializes raw inputs/QP/BSP/USB and starts both AOs without waiting for a host. Fault OFF precedes halt, without logging/reset/waits. Default shared hooks/identities remain unchanged for both original examples.
- Core 0/QV, PRIMASK, C++17, QP/C++ v8.1.5, QM 5.3.0, assertions and 100 Hz tick retained; core 1 unused. Fixed eight-entry AO queues, static events, no application heap/dynamic event pools.

QM numeric targets count transitions as well as states. The heartbeat still uses child indices 5/6, guarded by normalized source contracts and actual timing tests. The new relay machine counts its two parent transitions before IDLE/ON (indices 2/3); diagrams alone are not runtime proof.

## GPIO and electrical limits

| Resource | GPIO label | Function |
|---|---|---|
| IN1..IN4 | GP2, GP3, GP8, GP9 | Active-low; external 47 kOhm pull-ups, internal pulls disabled |
| REL1..REL4 | GP10..GP13 | Manually selected targets, at most one HIGH |
| Module LED | internal GP25 | Independent heartbeat |

GPIO labels are not physical pin positions. EXT1 GPIO23/24/25/29 positions are **GND on standard Pico 2**, not internal signals; do not use EXT1 GPIO25 for heartbeat. Unsafe LED overrides are compile-time rejected.

Tested silicon reports revision 2/A2: E9 applies. Earlier digital checks passed on all inputs, but active analog GPIO voltage was measured only on IN1 (about 0 V at nominal 5 V field excitation); IN2..4 low margins/minimum excitation/noise/tolerance remain unqualified. No automatic resistor/pull workaround is implemented.

All four relay bodies were user-confirmed default LEG-5F / 5 V without M/L suffix, nominal approximately 72 mA each at 25 C. Firmware still declares **250 mA USB power**, not 500 mA. Four nominal coils alone exceed it; maximum one coil remains the conditional budget case. Actual coil rail beyond L1/current/thermal/transient behavior during continuous ON remain quantitatively unverified. Source was Mac -> bus-powered Apple USB-C/HDMI/USB-A/USB-C adapter -> Pico USB. No capability is inferred from the connector or separate disconnected 0.5 A field supply.

## USB output and bounds

USB product `Pico 2 EVB Diagnostic`, VID/PID/board-unique serial and CDC layout are unchanged. Product/banner describe configured firmware, not automatic carrier detection. New command banner explicitly warns `No timeout.`; detected session loss/suspend still forces OFF.

```text
t=1240ms IN1=HIGH IN2=HIGH IN3=LOW IN4=HIGH
t=1260ms CTRL=ON RELAY=2 CMD=OFF,ON,OFF,OFF (no feedback)
t=1320ms CTRL=IDLE RELAY=0 CMD=OFF,OFF,OFF,OFF (no feedback)
```

These are illustrative rows, not hardware observations. `CTRL=IDLE/ON`, `RELAY=0..4` and the four command bits are software state, never physical contact/coil readback. The connection banner's command line is dynamic. Revision 2 means A2/E9; 3 means A3/A4 (not distinguished); unknown stays unknown.

CRLF lines; timestamps are monotonic sampling time, not USB delivery/electrical switching time. Fixed 256-byte in-flight line, one latest input/control sample and reply; <=16 RX/64 TX bytes and one completed command per poll. Partial lines finish intact; fully unsent updates may be replaced. Control/input reports outrank replies; TX backpressure never gates commands/OFF. Intermediate states may be missed/coalesced: not an event recorder or real-time guarantee. New sessions discard partial input/output and print a fresh banner/snapshot without automatic activation.

## Build and regeneration

From repository root, without flashing:

```bash
source env.sh
cmake -S . -B build/pico2 -G Ninja -DPICO_BOARD=pico2 \
  -DPICO_PLATFORM=rp2350-arm-s -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1
cmake --build build/pico2 --target qpcpp-pico-evb-diagnostic
python3 examples/pico-evb-diagnostic/tools/qm_codegen.py regenerate
python3 examples/pico-evb-diagnostic/tools/qm_codegen.py check
python3 examples/pico-evb-diagnostic/tools/qm_codegen.py diagrams
```

Artifact: `build/pico2/examples/pico-evb-diagnostic/qpcpp-pico-evb-diagnostic.uf2`. Normal Pico 2 builds produce three separate images; Pico 2 W retains only the two original examples. Normal builds consume stored generated C++ without QM. Confirm physical module variant before later flash; RP2350 boot label does not distinguish wireless boards.

Open `model/diagnostic.qm` in QM 5.3.0. `--qm PATH`/`QM_EXECUTABLE` may select the existing tool. Six generated files cover three machines; SVG/PNG exports go to `build/pico-evb-diagnostic-diagrams/`. Project-local checks reject wrong versions/stale/missing/extra files. Targets: `pico-evb-diagnostic-regenerate`, `pico-evb-diagnostic-check`. No tool installation.

## Automated validation

```bash
cmake -S examples/pico-evb-diagnostic/tests -B build/pico-evb-diagnostic-host \
  -G Ninja -DENABLE_QM_CHECKS=ON
cmake --build build/pico-evb-diagnostic-host
ctest --test-dir build/pico-evb-diagnostic-host --output-on-failure
python3 tests/bsp/verify_builds.py
python3 examples/pico-evb-diagnostic/tests/verify_builds.py
```

51 diagnostic cases (49 without QM checks), plus the existing 13 regression cases. Native address/undefined-behavior sanitizers default ON. Coverage includes all four latched channels past 20 simulated seconds, busy/duplicate/invalid rejection, correct/wrong-channel OFF, global OFF, matching/nonmatching pending cancellation, stale channel/generation handling, loss/suspend/reconnect, repeated fresh use without reset, bounded framing/reporting, TX backpressure/floods, actual transport/fault hooks, GPIO masks and unchanged heartbeat/input sampling. Five firmware images are checked for carrier/source/symbol/identity isolation.

Host-only idle/halt exceptions are not production paths. Native tests are **not** physical coil, USB-session, current, thermal, contact or Arm IRQ/transient qualification.

## Later hardware gates

Keep the board unpowered/unwired until separate flash/preparation/test approval. Later tests must verify the exact new image and its startup IDLE/all-OFF/inputs HIGH, plus physical heartbeat/dark relay indicators, **before** any ON command. Use Pico USB only, empty terminals and no external sources/modules. No app bytes during startup verification.

Plan an operator-present, explicitly reviewed short manual ON -> selected OFF observation first; do not interpret removal of the firmware timeout as approval for unattended/long thermal tests. Review each busy/wrong-OFF/global-OFF/session-loss/reuse test before execution. Match command bits with observed indicators/clicks without claiming contact feedback; remove sole USB power on unexpected behavior. End all-OFF, unpowered and unwired. No simultaneous coils, changed power source, field linkage or loads are authorized by this implementation.

A de-energized relay's NC contact remains closed. Neither dark LEDs nor software OFF/empty-contact bench observations establish a safety interlock, contact rating or permission for mains/load wiring.
