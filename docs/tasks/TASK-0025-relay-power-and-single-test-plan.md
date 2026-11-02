# TASK-0025 - Relay power research and single-relay test plan

## Status and approval

Research completed / one-shot behavior agreed; implementation delegated to TASK-0026.

The user approved verifying the fitted relay's coil current, USB power budget and a bounded single-relay test plan. No firmware/model/BSP changes, relay energization, flashing, power-on, contact/load wiring, tool installation or commit is authorized. Keep the board unpowered and all contact/field terminals empty.

## Objective

Resolve the actual fitted relay versus schematic naming discrepancy, evaluate the existing Pico-USB-only supply for one energized coil, and prepare an independently approved firmware/test milestone. Do not treat contact marketing ratings as proof of board-level load safety.

## User hardware evidence

After TASK-0024 cleanup (user confirmed board unpowered/all terminals unwired), the user reported these markings:

```text
recoy int'l leg 5f 15A 24V 5V DC | 15A 120V AC 50/60 Hz
```

The reported LEG-5F/5V DC appears consistent with the manual photograph noted in TASK-0021, unlike the schematic's RAS-0515 component name. Manufacturer spelling/precise ordering variant and whether all four relays match remain to be verified. Preserve the user's transcription; do not silently reinterpret it as a different certified part/rating.

## Approved scope

- Locate authoritative LEG-series manufacturer coil data, including coil voltage/resistance/current/power, tolerance, pickup/dropout and relevant temperature conditions. Distinguish source claims, nominal calculations and physical measurements.
- Recheck the existing board power path, relay driver/flyback components, USB descriptor/current assumptions and known supply readings (user-reported 3.3 V logic and 4.9 V VSYS with coils OFF).
- Evaluate one-coil power/current/transient margin without assuming a verified four-coil or safe load budget. Identify what is unknown about host/hub/cable, diode/ferrite/drop, actual coil and MCU current, and missing/unassembled bulk capacitance.
- Identify safely accessible coil-rail verification options, without asking for powered probing/board changes under this task.
- Plan a bounded, single-relay, empty-contact actuation: explicit request only, default OFF, automatic timeout/fault OFF, no PWM/heartbeat redirection, no input-driven activation, no automatic startup actuation. Exact behavior/model/commands require decisions and separate implementation approval.
- Preserve the independent copied Blinky HSM and diagnostic architecture; any later changes must follow model-as-source-of-truth and BSP GPIO isolation.
- Record remaining design choices as individual approval questions, not implicit implementation authorization.

## Planned files/artifacts

- This task record, `TASKS.md`, a relay research/planning report under `docs/hardware/`, and related hardware/open-point/example documentation where appropriate.
- Downloaded references/extracted text/inspection scripts only under ignored project-local `build/`; do not modify/install external tools or libraries.
- No production code/model/generated/CMake changes.

## Completion criteria

- Qualified source links/revisions and clear identification limits for the fitted part.
- Auditable one-coil current/voltage estimates and explicit unverified assumptions; no unsupported USB/current/contact safety guarantee.
- Paper test/firmware plan with startup/error/timeout/cooldown/busy behavior and power/test gates, or clearly stated remaining decisions.
- Separate implementation, flashing and hardware power/test approvals remain mandatory.
- No physical actuation or project source changes performed by this research.

## Results

Research findings and a candidate policy are recorded in `docs/hardware/relay-power-and-single-test-plan.md`. Board remains user-reported unpowered/unwired; no implementation/physical actuation performed.

Located a manufacturer-authored RAYEX LEG-F two-page PDF hosted by TME, corroborated by TME's exact Recoy/RAYEX LEG-5F listing. This also supports the user's Recoy branding transcription. The coherent default 5 V row specifies 70 Ohm +/-10%, approximately 72 mA / 0.36 W at 25 degrees C, maximum pickup 3.5 V and minimum dropout 0.5 V. Higher-power suffix variants have different currents and cannot be assumed compatible. The user subsequently confirmed all four relays are marked LEG-5F / 5 V DC with no additional M/L suffix. This supports the default-coil planning case; it is a user marking observation, not measured coil resistance/current or independent authentication.

Downloaded/inspected LRC's LMUN22xxLT1G series document hosted by iCEasy: the schematic driver LMUN2235 has 2.2 kOhm / 47 kOhm bias resistors and 100 mA collector absolute maximum. Saturation data at 10 mA do not guarantee the same voltage at a relay's higher current. Manufacturer website attempts were unavailable/parked, so the report explicitly identifies these as distributor-hosted manufacturer documents rather than direct live manufacturer-site verification. Hashes/source links and qualification limits are in the report. Native PDFKit extraction and rendered coil-table inspection used existing tooling; no installation occurred. Browser/raw research artifacts were relocated under ignored project-local `build/relay-research/`.

Important budget correction: TinyUSB's descriptor macro accepts milliamps and divides by two. The existing argument 250 therefore declares **250 mA**, not 500 mA. Confirmed in the actual diagnostic ELF CONFIG header (`09 02 4b 00 02 01 00 80 7d`, address `0x10017538`). Four nominal 72 mA coils alone exceed this budget (288 mA); one default coil is a conditional bench candidate, not a measured supply/thermal guarantee. Room-temperature nominal/tolerance, indicator/base contributions, typical-versus-maximum MCU current and unmeasured rail/USB path behavior are distinguished.

Candidate plan (not selected/implemented): REL1 only, explicit request after active USB session, one-second software pulse, automatic/fault/session-loss OFF, five-second cooldown with busy/cooldown rejection and no queued/replayed pulse train. Keep heartbeat independent, contacts/field terminals empty, no input-triggered activation. Software timers do not enforce a maximum under total CPU failure; no unapproved watchdog/auto-reset changes.

All-four marking consistency is now user-confirmed. The user identified the prior connection as the USB-A port on an Apple USB-C adapter/extender with HDMI, USB-A and USB-C ports, not a direct Mac USB-A connection. This resembles Apple's Digital AV Multiport adapter family; exact model/current specification is not established. The user confirmed the Apple adapter was powered only by the Mac, with no charger connected to its USB-C port. The planned source context is therefore Mac -> bus-powered Apple USB-C/HDMI/USB-A/USB-C adapter -> Pico USB. Do not infer an available-current guarantee or replace the 250 mA device declaration from this description.

Approved planning decision 1: the user accepted the first control milestone restricted to REL1, explicit manual USB request only, other relays OFF and no input-to-relay linkage. This is design approval only, not implementation or actuation authorization. Approved planning decision 2: the user accepted a fixed one-second ON pulse followed by automatic OFF, with no user-adjustable duration initially. Approved planning decision 3: the user accepted one pulse per boot, replacing the draft's repeatable cooldown. After completion the test stays locked OFF until a deliberate power cycle; terminal reconnect does not re-arm it. This prevents within-boot queued requests from generating a pulse train. No automatic reboot/reset is added. Approved planning decision 4: the user accepted an explicit OFF command that clears all relay outputs, cancels a pulse, and locks activation until power cycle, taking priority over pulse requests. This is software control, not a hardware emergency-stop guarantee. Next decision: detected terminal/session loss or USB suspend cancellation and reconnect behavior. Pulse/repeat/stop/session/model/protocol choices remain separate decisions. Actual source/cable current capability, loaded rail behavior and safely accessible coil-rail verification remain prerequisites for a later powered test. The user approved detected disconnect/session-loss/suspend cancellation: pending/running tests force OFF and lock; reconnect never replays/resumes. Before any request, a disconnected session merely keeps outputs OFF. They then instructed proceeding without further overdesign. The final agreed repeat policy is one pulse per boot, not the draft five-second cooldown. Implementation/testing is recorded in TASK-0026; flashing and hardware actuation still need separate approval. No actuation is approved by these planning decisions. Source/model/CMake/firmware artifacts were not modified, rebuilt or flashed by TASK-0025.
