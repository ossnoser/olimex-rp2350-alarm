# TASK-0028 - Individual manual relay commands

## Status and approval

Completed / automated checks and user-reported all-relay manual tests passed / power-off cleanup confirmed.

The user requested independently addressed relay ON/OFF commands and approved this final plan with "ok". During clarification the user initially chose one-second automatic OFF, then explicitly superseded it: "yes but remove the automatic off". The final approved policy has **no automatic relay timeout**. The user also chose one relay maximum, rejection rather than switching/queueing a new ON, reuse without power cycle, OFF on terminal/USB loss or suspend, and removal of `testall`.

This task authorizes documentation/software/native and firmware build validation only. Flashing, physical powered testing, commit and push require separate approvals. Board power-off/unwired cleanup was confirmed after TASK-0027; do not access hardware during implementation.

## Final command contract

- Exact lowercase newline commands `on N` and `off N`, N a single logical relay digit 1..4; plain `off` is global OFF. No GPIO-number, selected-duration, sequence, pulse, re-arm or all-ON command. Old `testall` and `test1` are invalid.
- `on N` is accepted only when all commanded outputs are OFF, the controller is IDLE, no ON is pending and the terminal session is active. It latches only N ON until explicit OFF, detected session loss/suspend or fault/power loss. **No timeout, gap, automatic sequencing or within-boot lockout.**
- Every ON while pending or any relay ON (including duplicate ON of that same relay) is rejected without switching, extending, queueing or disturbing it. Fresh ON commands are accepted after OFF/cancellation, with no reset.
- `off N` immediately clears only N and cancels a matching pending ON. It must not cancel another relay's pending ON or turn another active relay OFF. Already-OFF is harmless/idempotent.
- Plain `off` immediately forces all four OFF and cancels all pending ON requests, with priority ahead of queued activation. It does not permanently disable later fresh commands.
- Detected DTR/session loss, USB unmount or suspend immediately forces all OFF through the existing opted-in transport callback; invalidate pending requests/partial input by session generation. The model returns IDLE. Reconnect never activates anything: a new explicit command in a fresh active session is required.
- OFF is parsed software control, not a physical emergency stop. Serial backlog, USB/CPU failure can delay/prevent it. Unplug the sole Pico USB source on unexpected behavior. **A latched ON can remain ON indefinitely if the CPU/transport fails.** Empty-contact/operator-present diagnostics only, not an alarm/safety or contact/load-qualified controller.

## Scope and architecture

- Preserve raw input-change reports and independent 500 ms OFF/500 ms ON onboard heartbeat; no input-to-relay linkage or automatic startup activation.
- Retain two AOs/QV core0, priorities 2/1, static events and bounded eight-entry queues, no application heap/event pools, 100 Hz tick, 20 ms polls, 16 RX/64 TX bytes and one completed command per poll, 256-byte TX line and coalesced latest reports/reply. TX backpressure must never gate OFF/command execution.
- Replace sequence behavior in authoritative QM model with IDLE/ON, logical active relay 0..4, guarded selected-OFF and global/session OFF. Remove relay timer/timeout/gap/lockout behavior entirely, not just its CLI entry point. Retain modeled heartbeat and unused DUTY path unchanged. Recalculate/count numeric QM state targets and prove actual heartbeat timings.
- Immutable static channel events for ON/OFF with one generation-tagged pending activation record; revalidate owner-side before actuation and discard stale/cancelled events without disturbing unrelated active/pending channels.
- Carrier BSP exclusively owns GPIO/SDK calls. Existing checked single-relay activation clears other outputs first. Add checked selected-relay deactivation preserving other pins/command bits; global OFF/fault hooks remain unchanged. Report controller state, selected active relay and four commanded bits, never coil/contact feedback.
- Preserve original Blinky examples/models, shared default identities/session policies, SDK and QP submodule; no installations or changes outside project. Current 250 mA USB declaration remains: one nominal coil (~72 mA at 25 C) maximum, not four simultaneous coils.
- Preserve committed TASK-0027 baseline (`1a70535`) and exact previous sequence UF2/source in ignored `build/task28-baseline/` before modifications. Update example/hardware/task/index documentation with current vs historical scope clearly separated.

## Validation and acceptance

- Parser positive/negative cases for exact ON/OFF N and global OFF; reject missing/out-of-range/signed/multi-digit/trailing/uppercase/old sequence commands without affecting active outputs. Preserve framing/generation reset and poll/line bounds.
- Real QP/QV generated-model tests for every channel, latched ON well beyond the former one second/seven-second sequence durations, no timer-driven deactivation, duplicate/other-ON rejection, selected OFF vs wrong-channel OFF, repeated reuse without reset, no queued commands/replay, matching/nonmatching pending cancellation and stale events.
- Global OFF/session loss/suspend during pending/active/idle and with blocked TX, immediate command-mask clearing, fresh manual command only after reconnect. Independent heartbeat and raw input sampling continue during latched ON.
- SDK-stub carrier mapping/order tests: each active bit, deactivate same/different/inactive channel, invalid deactivation numbers (no pin changes), all-OFF/fault clear all command tracking; unrelated GPIOs preserved.
- Regenerate all six C++ files with existing QM 5.3.0, freshness/negative tests and three diagrams inspected. No hand edits to generated code.
- Run all native regression suites, five firmware builds and source/symbol/USB identity/descriptor isolation checks. Check whitespace and unchanged original examples/submodule.
- Record artifact hash/size and test results. No physical validation can be claimed for this new image from previous sequence observations. Continuous-coil loaded rail/current/thermal behavior, physical loss/fault and contact/load capabilities remain unqualified.

## Result

Implemented the final approved latched-control contract. At software completion no flash, serial command, hardware access, powered test, dependency installation, external SDK/submodule modification, commit or push had been performed. Subsequent separately approved flash/startup is recorded below.

### Implemented behavior

- Removed `testall`/`test1` command support, relay time event, timeout/gap/sequence logic and boot lockout. ON remains latched; there is no hidden one-second or seven-second automatic OFF.
- Commands are exact `on N`, `off N` (single digit 1..4) and global `off`. Busy/duplicate ON is rejected without GPIO effects. Selected OFF is idempotent, affects only its channel and cancels only its matching pending request. Global/session OFF cancels all pending activity and returns reusable IDLE.
- Static immutable ON/OFF channel events, one pending channel/generation record, owner-side connected/generation/IDLE/all-OFF validation. A cancelled old channel's notification neither consumes another channel's pending request nor clears an unrelated active relay. OFF still executes independently of TX capacity.
- Carrier selected deactivation is range-checked, clears only the corresponding GPIO/command bit and returns false/no changes on invalid ID. Activation and fault/global OFF preserve their previous semantics. GPIO mappings, input reporting, heartbeat, default shared BSP session behavior and 250 mA USB declaration are unchanged.
- New banner warns `No timeout.` (detected loss/suspend OFF remains). Status: `CTRL=IDLE/ON RELAY=0..4 CMD=...`. Software state/masks are not coil/contact feedback.
- Detected DTR/session/unmount/suspend events force all OFF; there is no inactivity or host-liveness watchdog. Closing a UI without dropping DTR/closing the port cannot be detected. CPU/undetected transport failure may leave an output ON indefinitely. This limitation is explicit in the example README; no guarantee for all communication failures is implied.

### Validation results

**64 automated cases passed**: diagnostic 51 (43 generated real-QP/QV control scenarios plus reporter/timing/offline/transport/carrier/contracts/freshness/negative-generation cases), original classic 4, original QM 6 and BSP 3. Native address/undefined-behavior sanitizers were enabled.

Each channel was held ON for over 20 simulated seconds with unchanged heartbeat/input sampling, then explicitly deactivated under blocked TX; this is native software evidence, **not a physical long-duration coil test**. Coverage includes duplicate/other-channel ON rejection, selected vs wrong-channel OFF, invalid/old commands while active, all four global/loss/suspend/generation cases, matching/nonmatching pending OFF, stale-channel notification, idle/offline behavior, thousand-command rejection flood, fresh post-reconnect/manual commands and repeated same-boot use across every channel. Actual shared transport callbacks/fault hooks and stubbed GPIO mapping/invalid-ID preservation passed.

All six generated files were regenerated normally with QM 5.3.0. Freshness and negative checks passed; the three latest state diagrams were exported to `build/pico-evb-diagnostic-diagrams/` and visually inspected. Heartbeat numeric targets remain 5/6; relay IDLE/ON are 2/3 after parent OFF/loss transitions. All five firmware images built; source/symbol/USB identity/descriptor isolation scripts passed. Original Blinky/model/submodule path diff was empty. `git diff --check` passed; final whitespace checks include the new task record.

Current diagnostic ELF: text **123440**, data **0**, bss **4168** bytes.

```text
New manual-control image (subsequently flashed with separate approval):
build/pico2/examples/pico-evb-diagnostic/qpcpp-pico-evb-diagnostic.uf2
SHA-256 788923645640755b97195f505338678ed8c3bd64838664fc67d65b9e7277ac08

Previous tested sequence rollback:
build/task28-baseline/source.tar
build/task28-baseline/four-relay-sequence.uf2
SHA-256 65010399c650ebc91a167196b605aeb7a21085d47c5e1cdd07ae8d91411e1856
Committed baseline: 1a70535
```

Build/test/codegen/diagram logs are ignored `build/task28-*.log`. The historical TASK-0026/0027 records remain unchanged; current example/hardware/board-support/index docs distinguish the new contract from previous observed one-shot firmware.

### Remaining gates

At software completion, separate flash approval was required before copying the validated image; that approval and flash were subsequently obtained as below. Read-only startup enumeration/banner/IDLE/all-OFF plus physical heartbeat/dark-indicator checks must precede separately approved, operator-present, empty-terminal manual ON/OFF testing. Keep the sole Pico USB removable; never assume an automatic OFF. Review short manual holds and selected/global/communication-loss/reuse observations before each powered test. Do not automatically run native 20-second holds on hardware.

No loaded coil rail/current/thermal measurement, physical selected/global OFF or new-image communication-loss/fault/contact qualification has been obtained. The board was user-confirmed unpowered/unwired at the prior milestone; no tool powered or accessed it during this implementation. No new-image physical results, further test authorization or commit approval are implied by software success.

## Approved flash and startup

Asked to approve flashing with no relay-start command to be sent, the user replied **"yes, device is ready"**. `/Volumes/RP2350/INFO_UF2.TXT` reported UF2 Bootloader v1.0 / Raspberry Pi RP2350 / Board-ID RP2350. This generic metadata does not distinguish wireless variants; the prior user-confirmed non-wireless Rev. A setup remains the basis.

Verified the exact image SHA-256 `788923645640755b97195f505338678ed8c3bd64838664fc67d65b9e7277ac08`, then copied using `COPYFILE_DISABLE=1 cp -X`. The diagnostic enumerated on **`/dev/cu.usbmodem142101`** (changed from the preceding test's 143101). IOUSB confirmed product `Pico 2 EVB Diagnostic` and board serial `9300C3F2E7952D15`.

Opened exclusively for four-second startup reading, raw 115200 with DTR/RTS and HUPCL. **Zero application bytes were written**; explicitly deasserted DTR/RTS and closed afterwards. New command banner, A2/E9 warning, IDLE/RELAY0/all-OFF and all four inputs HIGH were observed. Capture: `build/task28-startup-serial.txt` (414 bytes):

```text
Commands: on N / off N (N=1..4); off (all). No timeout.
t=15306ms CTRL=IDLE RELAY=0 CMD=OFF,OFF,OFF,OFF (no feedback)
t=15306ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
```

No `on`, selected/global `off` or sequence command was sent. No intended relay activation has occurred. The user confirmed **"yes"** to heartbeat blinking and all four relay indicators OFF. Physical startup observation passed. Next gate is separate permission for one short operator-present REL1 `on 1` -> `off 1` test, Pico USB only/all terminals empty, with explicit reminder that there is no automatic timeout. The user then replied **"yes"** to separate approval for that short manual REL1 ON -> OFF test. Authorized instructions: enter `on 1` once, observe for about two seconds, then enter `off 1` before replying; REL1 should remain ON until OFF, other indicators OFF throughout, then all OFF/IDLE. On any unexpected behavior remove sole Pico USB power. This is one short manual observation, not unattended/long continuous-coil qualification or permission to actuate other relays. Collect pasted console output and physical observations. No agent-controlled relay command has been sent. The user supplied the console result below; explicit physical confirmation remains pending.

## First manual REL1 ON/OFF trace

After the separately approved manual test, the user pasted the new A2/E9/no-timeout banner and:

```text
t=575006ms CTRL=IDLE RELAY=0 CMD=OFF,OFF,OFF,OFF (no feedback)
t=575006ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
ON requested: REL1 (manual OFF; no timeout).
t=580206ms CTRL=ON RELAY=1 CMD=ON,OFF,OFF,OFF (no feedback)
OFF requested: REL1 only.
t=597346ms CTRL=IDLE RELAY=0 CMD=OFF,OFF,OFF,OFF (no feedback)
```

The requested ON and selected OFF produced the expected software IDLE -> ON/REL1-only -> IDLE/all-OFF states. The sampled ON-to-OFF interval is **17140 ms**, rather than the suggested approximate two-second manual observation. Record the supplied timing honestly; it shows no reported one-/seven-second automatic OFF, but is not precision GPIO/contact timing or continuous-current/thermal qualification. No changed input record appears in the pasted text; unsampled transients are not excluded.

Asked whether REL1 physically stayed ON until `off 1`, then turned OFF with all other relays OFF throughout, the user replied **"yes"**. The first approved manual REL1 latched ON/selected-OFF test passed its user-observed behavior and software trace. This does not qualify contacts, precision timing, loaded current/rail, thermal margins or fault paths.

Further relay, busy/wrong-OFF/global/loss/reuse tests and power-off cleanup remain pending and need reviewed permission; no retry or additional actuation is implied. Next proposed separately approved check: without resetting/changing terminal session, `on 1` -> `on 2` (must reject with REL1 unchanged) -> `off 2` (must preserve REL1) -> `off 1` (all OFF/IDLE). All steps must finish before replying; unplug sole USB on any unexpected activation/state. Do not execute this proposal without user permission.

## User-reported broader manual checks

Responding to that proposal, the user reported: **"OK, I tested all the relay. All good. It is not possible to activate 2 reley. off switch off any reley on."** Record this as user-performed all-four relay control checks, observed prevention of a second simultaneous activation, and global `off` successfully deactivating whichever relay was ON. No agent-controlled command or additional powered test was performed.

No additional console trace, exact relay-pair/duration matrix or explicit wrong-channel `off N` observation was supplied. Do not claim the proposed `off 2`-while-REL1-ON step was independently confirmed: that distinction remains automated-test evidence only unless the user clarifies otherwise. Physical communication-loss/suspend/fault paths, loaded rail/current/thermal limits and real contacts remain unqualified. The limited manual-control milestone passed the reported behaviors, not every physical failure case.

Asked to unplug Pico USB, leave external supplies disconnected/terminals empty and confirm the board unpowered, the user replied **"done"**. Unpowered/unwired cleanup is user-confirmed; this closes the limited manual-control milestone. No additional actuation/reset/retry or commit/push was authorized merely by receiving this result.

## Commit authorization

After completion and confirmed cleanup, the user explicitly requested **"commit changes"**. This authorizes committing the TASK-0028 model/generated code, individual relay commands/BSP deactivation, tests and documentation/observation records. Ignored build/research/rollback artifacts remain excluded. No push or further hardware action is authorized. The resulting Git commit identifier is reported to the user after success. No new-image physical relay, continuous-current/thermal, session-loss/fault/contact result or commit/push authorization is implied by startup success.
