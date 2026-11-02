# TASK-0026 - REL1 one-shot implementation

## Status and approval

Completed / automated checks and initial REL1 pulse passed / final cleanup confirmed after TASK-0027.

After accepting REL1-only manual USB control, a fixed one-second pulse, one pulse per boot, explicit OFF-and-lock, and detected USB/session-loss cancellation, the user accepted the final policy and instructed: "ok, but dont't overting about this test. Let's go." This is treated as instruction to proceed with the agreed software implementation/testing, not permission to flash or energize hardware. TASK-0025 documents the supporting research. No further design-question round is needed for implementation details below.

## Objective

Extend the independent Pico 2 EVB diagnostic with the smallest explicit REL1-only test. Preserve heartbeat/input reporting, keep all other relays OFF, never activate at startup or from inputs, and prevent repeated/queued/reconnected activation within a boot.

## Behavior

- Exact newline-terminated commands `test1` and `off`, no echo or adjustable pulse length.
- `test1`: accepted only while READY and in an active, configured USB terminal session. Fixed one-second ON, then OFF and LOCKED until deliberate power cycle/reset.
- Reject repeated requests while active or locked; no cooldown/replay/re-arm command. Terminal reconnect cannot clear LOCKED.
- `off`: force all outputs OFF, cancel timer/request and lock, including when issued before the first pulse. Prioritize cancellation over queued requests.
- Detected USB loss/suspend/terminal session boundary forces physical outputs OFF; cancel/lock a pending/running request. Loss before any request leaves outputs OFF and permits only a later fresh-session request. Never replay stale session commands.
- Keep fail-stop OFF action and no automatic-reset policy. Software timing cannot guarantee a maximum ON period under total CPU failure; contacts remain empty in any later test.
- Report commanded output state explicitly, not contact feedback. Nonblocking TX must not delay OFF.

## Implementation details within agreed scope

- Keep two AOs: copied Blinky AO owns an explicit generated RelayTest QHsm with READY/PULSING/LOCKED states and a separate time event; existing running/off/on heartbeat timing remains independent. Diagnostic AO handles bounded command parsing, input reporting and USB service.
- Model remains authoritative; generated C++ is never hand edited. Extend codegen/freshness/diagram checks for the added model class.
- Only carrier BSP can drive REL1 GPIO HIGH; no general relay-ON interface for channels 2..4. Retain early all-OFF initialization.
- Add an optional shared-BSP session-loss action, default disabled for existing examples, including suspend handling/session invalidation only for opted-in diagnostic. Preserve original Blinky behavior/identity.
- Retain eight-entry AO queues, 256-byte transmit line buffer, 16 RX/64 TX bytes per 20 ms sample poll, static events and no application heap/event-pool allocation. Replies/status may coalesce under backpressure.
- Save the validated pre-extension example/BSP and UF2 under ignored project-local build storage before changing sources; no commit is authorized.
- Update test/docs/task index and record any limitations. No dependencies/tool upgrades or modifications outside the project.

## Validation and acceptance

- Host-test generated relay HSM with real QP/QV: no boot activation, only REL1, fixed timeout independent of heartbeat, repeated requests rejected, OFF from all states, cancellation priority, disconnect/suspend/session changes, stale requests, no re-arm on reconnect, and no input linkage.
- Reporter/parser bounds, malformed commands, partial output/coalescing, generation reset and no control delay due to TX backpressure.
- Carrier/session-loss/identity hook tests using SDK stubs; original examples remain unlinked to carrier and retain defaults.
- QM generation/freshness/negative cases and inspected diagrams. Preserve baseline heartbeat state behavior while intentionally extending the copied model; original models unchanged.
- Build Pico 2 diagnostic and the four existing firmware combinations; rerun current automated regression suites and build inspections. Hardware observations are not inferred from host tests.
- Review source/rail requirements before any later actuation; current firmware advertises 250 mA and only one default approximately 72 mA coil is a conditional candidate. Source/cable current and loaded rail remain unmeasured.
- Separate flashing and physical test approval required. Board stays unpowered/unwired during implementation; no GPIO manipulation via tools or device connection performed.

## Results

Implemented the agreed `test1` / `off` interface, generated RelayTestHsm owned by BlinkyAO, independent one-second timer, one-per-boot lockout, priority cancellation and session-generation validation. Existing heartbeat and input reporting are preserved. Other channels expose no ON API. An atomic carrier command latch reports writes, never contact feedback.

Opted-in transport callbacks clear outputs on session boundaries/suspend, invalidate device buffers/generation, and leave original examples' default behavior unchanged. Diagnostic service detects even a suspend/resume between polls and informs the controller. A pending/running request is cancelled/locked; before any request an unused test stays READY. The device rejects already-submitted stale-generation requests and clears partial/device-buffered commands. It cannot authenticate when a host typed data or prevent a client from submitting a new complete command later: use a trusted manual terminal, close it before flashing, and do not stream/reconnect-automate commands.

Baseline preserved before source modification: `build/task26-baseline/source.tar` and `relay-off.uf2` (SHA-256 `c7dabfed7a3621afd4cca2ca96e7813fcb83badcbc562de74f6d41912bba96c6`). These are ignored local rollback artifacts, not a commit.

Validation:

| Check | Result |
|---|---|
| Diagnostic CTest suite, QM enabled | 18/18 passed |
| Existing handwritten example | 4/4 passed |
| Existing QM example, QM enabled | 6/6 passed |
| Existing BSP LED adapters | 3/3 passed |
| Original four firmware builds plus extended diagnostic | All five built and passed source/symbol/identity isolation |
| USB budget in all actual ELF CONFIG headers | 125 units = 250 mA; unchanged |
| Generator output | Six files byte-identical to QM 5.3.0; negative checks passed |
| Diagrams | All three exported/visually inspected; transition glyphs corrected in model |
| Original example/model/submodule changes | None (`git diff --exit-code HEAD -- examples/blinky examples/blinky-qm third_party/qpcpp`) |
| Whitespace | `git diff --check` passed |

**31 automated cases passed**, including ten real-QP/QV relay scenarios: timeout, OFF before/while/after pulse, pending OFF/loss, stale generation, active loss, pre-request loss, suspend/resume, flood/backpressure and unchanged heartbeat. Real transport callbacks are separately exercised with a TinyUSB stub; actual fault/timer-start hooks and REL1-only GPIO/command-latch paths use SDK stubs. Native sanitizers remain enabled. None of these execute firmware on the device.

Implementation issues caught before release: QM-generated local `status_` shadowed a proposed model field (renamed in model); added parent transitions changed QM numeric child indices and initially prevented heartbeat entry. The unchanged-behavior runtime tests caught this. QM requires transitions before child states, so model targets were correctly renumbered to off/on indices 5/6 and contracts now check both semantic target indices and preserved phase bodies. Generated code was always regenerated, never patched. Offline tests were updated to reflect that no commands are consumed while disconnected and real transport flushes old device RX at a session boundary. These are resolved, not waived failures.

Artifact: `build/pico2/examples/pico-evb-diagnostic/qpcpp-pico-evb-diagnostic.uf2`.

- SHA-256: `6907e91286fee6d96c8f83834a387d4c5ce7245a0d7c0b4ac738eede209950fe` (recheck after rebuild).
- Linker-reported sections: text 122264, data 0, bss 4188 bytes; not a full stack/heap budget.
- Existing TinyUSB macro-redefinition warnings remain; no unrelated USB configuration change was made.

Current example README documents commands, ownership, buffer/session semantics and physical gates. Hardware remains user-reported unpowered/unwired from the previous cleanup; no port was opened and no flash, power-on, relay actuation, commit or push was performed during this task. The user subsequently answered "go" to the explicit request to flash this new UF2 without sending relay commands. Flashing is now approved; rail verification and actuation remain separately gated. The selected UF2 hash was rechecked as `6907e91286fee6d96c8f83834a387d4c5ce7245a0d7c0b4ac738eede209950fe`. Initially `/Volumes/RP2350` was not present, so no copy occurred. The user was instructed to close the Pico terminal, keep external sources/terminals disconnected, unplug Pico USB and enter BOOTSEL manually, then reported ready.

## Approved flash and startup verification

Rechecked the exact new UF2 hash `6907e91286fee6d96c8f83834a387d4c5ce7245a0d7c0b4ac738eede209950fe` and `/Volumes/RP2350/INFO_UF2.TXT` (UF2 Bootloader v1.0, Model Raspberry Pi RP2350, Board-ID RP2350). As before, this generic ID does not distinguish wireless boards; the known user-confirmed non-wireless module is the target.

Copied using `COPYFILE_DISABLE=1 cp -X ... /Volumes/RP2350/`. The copy succeeded. A first three-second check briefly still saw the boot-volume path while CDC was already present; a follow-up confirmed the volume had disappeared and no disk remained at that path. The USB product was `Pico 2 EVB Diagnostic`, configured allocation 250 mA; registry board serial matched the prior unit. The enumerated port changed to `/dev/cu.usbmodem143101`.

As read-only startup verification, opened the sole CDC port at 115200/DTR, captured for three seconds, then dropped DTR/restored settings/closed it. **No application bytes, `test1`, or `off` command were sent.** Capture: ignored `build/task26-startup-serial.txt`, 391 bytes. It confirms the new command-capable build:

```text
Pico 2 EVB Diagnostic / configured PICO-EVB Rev. A
RP2350 raw revision=2 (A2)
WARNING: GPIO E9 applies; validate input voltage margins.
Relay commands: OFF OFF OFF OFF (no contact feedback)
Inputs: HIGH=inactive, LOW=active; raw/unfiltered
Commands: test1 (1s, once/boot); off (cancel+lock).
t=76446ms REL1=READY CMD=OFF (no feedback)
t=76446ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
```

Flash/enumeration/new-banner startup checks passed. READY is unused; metadata-only terminal close must not re-arm a used test or itself activate anything. The user subsequently confirmed the onboard heartbeat is blinking and all four relay indicators are OFF after the flash. This is a visual observation, not contact/transient feedback. At that startup checkpoint, actual coil-rail verification and explicit actuation permission were pending; later authorization and first-pulse observations are recorded below. Next proposed preparation is a safely accessible EVB REL1 flyback-diode D1 cathode/+5V rail measurement against system GND, with USB unplugged for probe placement/removal; stop if the actual point cannot be identified/accessed safely. No relay energization command had been sent at startup verification.

## First single-pulse authorization

Asked for the actual coil-rail voltage/check, the user replied: "It is all ok go on with the test". Record this as the user's confirmation to proceed and explicit permission for one manual REL1 pulse under the existing empty-terminal/Pico-USB-only conditions. **No numeric coil-rail reading was supplied or independently verified**; retain that measurement/current/transient limitation rather than inventing a voltage or declaring the rail quantitatively qualified.

Next instruction: remove measurement probes safely, use only Pico USB with all input/contact terminals/modules disconnected, verify READY/OFF, enter `test1` once in the existing diagnostic terminal. Do not power-cycle/retry if already LOCKED. Observe REL1 for a nominal one-second pulse followed by OFF/LOCKED, all other indicators OFF; disconnect sole USB power on unexpected behavior. The user performed the one permitted manual `test1` request and replied "It worked" to the requested observation (REL1 active for about one second then OFF, others OFF), supplying this trace:

```text
t=4821ms REL1=READY CMD=OFF (no feedback)
t=4821ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
test1 requested (1 second, once per boot).
t=15441ms REL1=PULSING CMD=ON (no feedback)
t=16421ms REL1=LOCKED (power cycle required) CMD=OFF (no feedback)
```

The pasted banner also confirmed revision 2/A2 and the new command interface. First empty-contact REL1 pulse passed user-observed behavior and the expected READY -> PULSING -> LOCKED/OFF software sequence. The 980 ms interval between sampled PULSING and LOCKED records is not measured GPIO/contact ON duration: the first ON sample can occur after the actual entry/action. Do not treat it as a timing failure or a precision one-second measurement. No changed input report appears in the supplied trace; this does not exclude unsampled transients.

No agent-controlled relay command was sent. Actual loaded coil voltage/current/contact state remain unmeasured. Next proposed negative check is one additional `test1` request **without reset/power cycle**, solely to verify LOCKED rejection with no further activation. Unexpected activation means unplug USB and stop; no repeat/power-cycle/re-arm attempt is authorized by that check. The user was then asked to issue one additional `test1` without resetting/power-cycling, only to check LOCKED rejection and to stop on any unexpected activation. They reported **no relay activation**. This matches the intended within-boot lockout, but no rejection-text trace was pasted; do not invent a specific received reply. No agent-controlled command was sent and no second pulse was reported.

Physical OFF-during-pulse/session-loss/suspend/fault tests remain unperformed; their current evidence is automated testing only. Final checkpoint: disconnect Pico USB, leave external field sources/modules disconnected and all terminals empty, then obtain user confirmation before closing the limited hardware smoke milestone. The user instead requested the TASK-0027 extension at that point; after its successful unloaded sequence, the user explicitly confirmed USB disconnected, all supplies disconnected/terminals empty and board unpowered. Final cleanup is therefore confirmed after TASK-0027, not retroactively assumed at the earlier checkpoint. Coil/board current and loaded voltage remain unmeasured, the USB allocation remains 250 mA, and software timing is not an independent hardware maximum/fail-safe guarantee.
