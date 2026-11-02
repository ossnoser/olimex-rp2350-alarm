# TASK-0027 - Four-relay one-shot sequence

## Status and approval

Completed / automated checks and approved unloaded four-relay sequence passed / power-off cleanup confirmed.

The user explicitly requested: REL1 OFF -> ON for one second -> OFF, wait one second, repeat for REL2, REL3 and REL4, then stop. The agent clarified that one manual `testall` runs the whole sequence and asked whether to retain one complete sequence per boot. The user selected "For now lockout." This approves the requested software extension with that repeat policy; no additional design round is needed. Flashing, physical actuation and commits remain separately gated.

## Objective and exact behavior

Replace the REL1-only command with `testall`, preserving the existing independent heartbeat/input diagnostic and safety policies:

| Nominal interval after accepted request | Commanded outputs |
|---|---|
| 0..1 s | REL1 ON only |
| 1..2 s | All OFF |
| 2..3 s | REL2 ON only |
| 3..4 s | All OFF |
| 4..5 s | REL3 ON only |
| 5..6 s | All OFF |
| 6..7 s | REL4 ON only |
| After 7 s | All OFF, LOCKED |

- All OFF initially; no automatic startup/input-driven activation.
- One accepted `testall` starts the entire sequence, not just one relay. No repeat, selected-relay or adjustable-duration command; old `test1` becomes invalid.
- Once a sequence is accepted, requests cannot restart/extend/queue another sequence within that boot. Completion/cancellation locks OFF until deliberate reset/power cycle, not terminal reopen. No automatic reset.
- `off` remains unconditional software cancellation/all-OFF-and-lock, with priority over queued requests, including before the first sequence or during any gap.
- Existing opted-in USB/session/suspend OFF callback and generation checks remain. Loss cancels/locks a pending/running sequence, including its all-OFF gaps; never resume on reconnect. An unused READY test remains OFF until a fresh request.
- Maximum one commanded ON at a time. Empty contacts, no field input linkage or auxiliary carrier sources. Current USB descriptor remains 250 mA; four coils are sequential, never simultaneous.
- Timing uses the existing 100 Hz framework tick and is nominal, not an independent hardware bound under CPU/timer failure.

## Implementation scope

- Extend the existing generated RelayTestHsm with explicit pulse/gap/locked behavior and sequence step; keep its timer owned by BlinkyAO. Both 500 ms LED phases and DiagnosticAO's 20 ms sampling remain independent.
- Extend carrier BSP activation to a checked relay number 1..4, clearing every output before enabling the selected pin. No GPIO numbers/SDK calls in application/model code.
- Preserve bounded `testall`/`off` parser, static events/one pending generation-tagged request, eight-entry AO queues, 256-byte TX buffer, one command/16 RX bytes/64 TX bytes per poll, nonblocking coalesced reports and no application heap/event pools.
- Report selected sequence step, READY/PULSING/GAP/LOCKED and the four commanded output bits explicitly, never contact feedback.
- Update QM model, regenerate all six generated files normally, inspect diagrams, extend positive/negative generation checks and tests. Do not hand-edit generated code or alter original Blinky examples/models/framework versions.
- Preserve TASK-0026 source and UF2 in ignored project-local rollback storage before modifications.
- Update task/index/example/hardware documentation. No installations, source changes outside project, external SDK/submodule changes or commits.

## Validation / completion criteria

- Real QP/QV native tests verify exact R1/R2/R3/R4 ordering, 100-tick ON durations and 100-tick all-OFF gaps, final lockout, at most one output ON, no startup/input-driven activation and unchanged heartbeat/sampling.
- Verify OFF/loss during each pulse and gap, pending/stale commands, repeat rejection, reconnect/no re-arm, suspend/resume and TX backpressure. No buffered request train or continuation after abort.
- SDK-stub carrier tests verify correct four-pin mapping, one-bit activation, invalid relay rejection/all-OFF and callback/fault command-mask clearing; existing examples retain default shared-BSP behavior/identity and no carrier dependency.
- Parser accepts only exact `testall` and `off`; malformed/old selected-relay commands cannot activate anything. Command execution/OFF do not depend on TX space.
- Fresh generated code/negative fixtures and three diagrams; all current regression suites and five firmware builds/identity/budget inspections pass.
- Record hardware evidence honestly: TASK-0026 first REL1 pulse/repeat observations do not validate the new sequence. Coil current/loaded rail/transients and real contact/load ratings remain unmeasured/uncertified.
- No flash, physical power/relay test or commit until separately approved.

## Starting hardware state and evidence

TASK-0026 completed a user-observed one-second REL1 pulse and a repeat request with no further activation. The agent requested power-off cleanup; instead the user requested this extension, so that cleanup state was not independently confirmed. Instructions for this implementation are to unplug Pico USB and leave every terminal/external source/module disconnected. No agent-controlled hardware command is part of implementation.

All four bodies were user-confirmed LEG-5F / 5 VDC, no M/L suffix; manufacturer default approximately 72 mA per coil at 25 degrees C is a planning estimate. Maximum one coil remains the conditional USB-budget case. No larger/simultaneous load budget is inferred.

## Result

Implemented the agreed fixed sequence and lockout without changing original Blinky sources/models, external SDK or QP submodule. At software completion, no new installation, flash, serial command, physical sequence, commit or push had been performed. Subsequent separately approved flash/startup evidence is below.

### Software evidence

- `testall` replaces `test1`; the old command is invalid rather than an alias for a larger actuation. `off` remains cancellation-and-lock.
- QM RelayTestHsm now has READY/PULSING/GAP/LOCKED and checked logical step 1..4. Pulse timeout chooses GAP only below step 4; GAP timeout advances the step and enters PULSING; fourth pulse exits directly to LOCKED/all-OFF. USB loss and OFF abort every phase, with no re-arm on reconnect.
- Carrier activation checks relay range, clears every coil before selecting one GPIO, tracks a lock-free four-bit command mask and returns false/all-OFF for invalid numbers. The HSM faults OFF on impossible selection.
- USB reports `SEQ`, `STEP` and all four commanded bits. STEP during GAP is the just-completed relay, not an ON indication. Nothing is contact feedback.
- All six generated files regenerated via QM 5.3.0; freshness/negative tests passed. All three latest SVG/PNG diagrams exported to `build/pico-evb-diagnostic-diagrams/` and visually inspected, with multiline pulse action kept within its state box. Heartbeat numeric state targets remain 5/6.
- **50 automated cases passed**: 37 diagnostic (29 sequence/abort policy scenarios plus parser/reporter/timing/transport/carrier/contracts/generation checks), original classic 4, original QM 6, shared BSP 3. Native sanitizers enabled. Sequence tests assert exact eight mask-change records over 700 ticks: ON/OFF R1, ON/OFF R2, ON/OFF R3, ON/OFF R4 at 100-tick spacing; no last gap or second run. OFF/session loss/between-poll suspend are tested in every one of the seven phases, plus pending/stale requests, queued-gap timeout cancellation, floods/backpressure, reopens and independent heartbeat.
- Five firmware builds and source/symbol/USB identity isolation passed: three Pico 2 images and the two unchanged Pico 2 W examples. `git diff --check` passed; `git diff --name-only -- examples/blinky examples/blinky-qm third_party/qpcpp` was empty.
- Current diagnostic ELF: text **123088**, data **0**, bss **4192** bytes. No descriptor/budget change: still 250 mA and maximum one commanded coil.

### Artifacts

New image:

```text
build/pico2/examples/pico-evb-diagnostic/qpcpp-pico-evb-diagnostic.uf2
SHA-256 65010399c650ebc91a167196b605aeb7a21085d47c5e1cdd07ae8d91411e1856
```

Immediate previous REL1-only rollback:

```text
build/task27-baseline/source.tar
build/task27-baseline/relay1-one-shot.uf2
SHA-256 6907e91286fee6d96c8f83834a387d4c5ce7245a0d7c0b4ac738eede209950fe
```

Build/codegen/diagram logs are under ignored `build/task27-*.log`. Earlier relay-OFF rollback is still under `build/task26-baseline/`.

### Remaining gates and limitations

The user subsequently approved flashing this exact image with "go". Flash/startup completed as below. Separate permission and physical startup observations remain needed before its first empty-contact sequence. Start/field/contact terminals, external modules/supplies and carrier USB-C must remain disconnected; Pico USB only. New startup must be verified commanded all-OFF/READY and physically dark relay indicators before any `testall` request. Do not send `off` as a status probe: that consumes READY and locks the boot.

The prior REL1-only user observations do not validate the new image, all four as-built mappings, coil current/loaded rail/timing/transients, electrical contact state or physical OFF/session-loss/fault paths. No numeric coil-rail value was provided for TASK-0026. At software completion the new sequence had no physical evidence; subsequent approved observations are recorded below. Boot/watchdog/debugger/brownout behavior and CPU-failure ON duration remain unqualified. No power-off cleanup confirmation was received during software implementation; no tool accessed hardware during that software milestone.

## Approved flash and startup verification

After the agent explicitly requested approval to flash (with no start command to be sent), the user replied **"go"**. `/Volumes/RP2350/INFO_UF2.TXT` reported UF2 Bootloader v1.0 / Raspberry Pi RP2350 / Board-ID RP2350. Generic boot metadata cannot distinguish wireless variants; the existing physical non-wireless/Rev. A identification remains the basis.

Rechecked image SHA-256 `65010399c650ebc91a167196b605aeb7a21085d47c5e1cdd07ae8d91411e1856`, then copied using `COPYFILE_DISABLE=1 cp -X`. A short-lived boot-volume directory remained visible during filesystem lag, but the new diagnostic enumerated on `/dev/cu.usbmodem143101`. IOUSB confirmed product `Pico 2 EVB Diagnostic`, board serial `9300C3F2E7952D15`.

Opened exclusively for startup observation, configured raw CDC/DTR, read four seconds with **zero application bytes written**, then closed. The new `testall` banner and READY/STEP1/all-OFF state were observed; raw inputs all HIGH, silicon revision 2/A2. Capture: `build/task27-startup-serial.txt` (415 bytes):

```text
Commands: testall (R1..R4, once/boot); off (cancel+lock).
t=32466ms SEQ=READY STEP=1 CMD=OFF,OFF,OFF,OFF (no feedback)
t=32466ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
```

No `testall` or `off` was sent, no intended coil activation occurred and no test lockout was deliberately consumed. The user then confirmed the heartbeat was blinking and all four relay indicators were OFF. Asked separately to approve one complete unloaded `testall` sequence (Pico USB only, all terminals empty), the user replied **"yes"**. This authorizes one manual full R1..R4 sequence, not automatic retries, reset/re-arm, simultaneous coils or contact/load wiring.

Next instruction: with the diagnostic terminal showing READY and all commands OFF, enter `testall` once. Observe R1 -> R2 -> R3 -> R4, one nominal second ON each, one second all-OFF between relays, then all OFF/LOCKED after about seven seconds. Stop/unplug sole USB power on unexpected behavior; do not reset/retry. Collect pasted control records and user-observed order/indicators. No agent-controlled start command has been sent. The user subsequently supplied the sequence trace below; physical order/indicator confirmation remains pending. Physical cancellation/fault tests are unperformed; software command flags are not electrical/contact feedback.

## First manual sequence trace

The user pasted the new banner (revision 2/A2, `testall` interface) and these records after the authorized manual sequence:

```text
t=197126ms SEQ=READY STEP=1 CMD=OFF,OFF,OFF,OFF (no feedback)
t=197126ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
testall requested (R1..R4, once per boot).
t=203266ms SEQ=PULSING STEP=1 CMD=ON,OFF,OFF,OFF (no feedback)
t=204246ms SEQ=GAP STEP=1 CMD=OFF,OFF,OFF,OFF (no feedback)
t=205246ms SEQ=PULSING STEP=2 CMD=OFF,ON,OFF,OFF (no feedback)
t=206246ms SEQ=GAP STEP=2 CMD=OFF,OFF,OFF,OFF (no feedback)
t=207246ms SEQ=PULSING STEP=3 CMD=OFF,OFF,ON,OFF (no feedback)
t=208246ms SEQ=GAP STEP=3 CMD=OFF,OFF,OFF,OFF (no feedback)
t=209246ms SEQ=PULSING STEP=4 CMD=OFF,OFF,OFF,ON (no feedback)
t=210246ms SEQ=LOCKED (power cycle required) STEP=4 CMD=OFF,OFF,OFF,OFF (no feedback)
```

Software observations match the requested R1 -> R2 -> R3 -> R4 sequence, all-OFF gaps, at most one commanded ON, and final STEP4 LOCKED/all-OFF. After the first observed ON row, subsequent boundaries are 1000 ms apart. The first ON-to-GAP record interval is 980 ms, consistent with the 20 ms sampler observing ON after its actual entry; the trace is not an instrumented GPIO/contact timing measurement. No changed input record appears in the supplied text; unsampled transients are not excluded.

Asked explicitly whether the physical relays activated in order 1 -> 2 -> 3 -> 4, each ON for about one second with one-second all-OFF gaps, the user replied **"yes"**. The first approved unloaded four-relay sequence therefore passed the requested user-observed order/pattern and the expected software trace ending STEP4 LOCKED/all-OFF. These observations are not instrumented contact/timing/coil-current feedback or load/safety qualification.

No new-image physical negative repeat request or abort/loss/fault test was performed; their current evidence is automated testing (the earlier REL1-only repeat observation belongs to TASK-0026). Loaded rail/current/transients and power-off cleanup remain unconfirmed. No retry/reset/new actuation is authorized by this success. Final instruction was to unplug Pico USB, leave all external supplies/modules disconnected and terminals empty, and confirm the board was unpowered. The user replied **"confirmed"**. Power-off/unwired cleanup is user-confirmed; this closes the limited unloaded-sequence milestone. At cleanup confirmation, no further actuation, commit or push was authorized/performed.

## Commit authorization

After completion and cleanup, the user requested **"commit the changes"**. This separately authorizes committing the pending TASK-0021..0027 integration, implementation, generated code, tests and observation records together. Ignored build/research/rollback artifacts remain excluded. No push or additional hardware actuation is authorized. Git records the resulting commit identifier; it is reported to the user after success.
