# TASK-0023 - IN1 low-voltage input pilot

## Status and approval

Approved / completed (single-channel nominal 5 V functional pilot).

The user approved documenting and performing an IN1-only low-voltage test, one step at a time. They subsequently corrected the source to a fixed **5 V DC** supply and explicitly declined the proposed external resistor, citing a **0.5 A** supply rating. This amends the pilot to direct 5 V excitation of the protected IN1 DC terminal only; it does not authorize any firmware/pull changes, relay actuation, other inputs or mains work.

## Objective

Using the existing TASK-0022 output-only diagnostic, test the IN1 optocoupler path and HIGH -> LOW -> HIGH response on the user's A2 Pico 2 / PICO-EVB Rev. A. Measure GPIO voltage rather than assuming a digital LOW proves adequate margin. Keep all relay commands OFF and contact terminals empty.

## Equipment and baseline

- Fixed **5 V DC, 0.5 A** supply according to the corrected user description, not a settable-current-limit bench supply. User confirmed approximately +5 V measured directly across its output, disconnected from the EVB, under the requested positive/negative probe convention.
- **No external series resistor**, per explicit user instruction. The earlier offered 2.2 kOhm resistor is not used/measured in this revised pilot.
- Multimeter in DC voltage mode, red lead in V socket. Never use current/ohms mode across powered rails.
- Existing diagnostic and Pico-USB-only carrier power. TASK-0022 recorded 3V3(OUT)=3.3 V, VSYS=4.9 V, inactive input GPIOs approximately 3.27 V and corrected relay GPIOs 0 V. Normal power-cycle and input-HIGH reporting passed initial limited smoke checks.
- SDK silicon revision 2/A2; RP2350-E9 applies. Active optocoupler sink/LOW margins are not yet validated.

## Source-qualified routing

See [the Rev. A investigation](../hardware/pico-evb-rev-a-integration.md) and the pinned vendor manual p. 8, re-inspected for this pilot. IN1 schematic terminal 2 is the low-voltage **30VDC/DC** path, terminal 3 its field-side **COM**, terminal 1 the AC path, unused here. The vendor specifies 3.3..30 VDC at the DC input. The 30VDC marking is not an instruction to supply 30 V.

Confirm the actual connector/silkscreen before wiring. Numbers identify schematic terminals, not a guessed left/right orientation. Do not confuse input COM with relay-contact COM or Pico/system GND.

IN1 drives GP2, Pico physical pin 4. GPIO labels and physical pin numbers differ. GPIO voltage measurements reference **system GND**, not isolated field COM. Do not add a field-COM/system-GND jumper; the optocoupler does not require a shared MCU ground.

## Current limiting and limitations

The schematic DC path already includes approximately **3.9 kOhm**, an indicator LED and an optocoupler LED. An additional resistor is not required by the vendor for a correctly wired 5 V DC input. Even neglecting LED drops, 5 V / 3.9 kOhm is approximately 1.28 mA; actual normal input current is lower. This is an estimate, not a measured current/CTR guarantee.

**0.5 A is supply capacity, not evidence of a safe adjustable fault-current limit.** The onboard resistor limits current only through the intended input path. It does not protect miswiring/shorts that bypass it. The previously proposed external resistor would have provided additional downstream fault limitation; the user has declined it. Do not interpret the 0.5 A rating as equivalent protection.

At 5 V the optocoupler's low-current behavior and A2/E9 still require actual measurements. Removing the additional resistor increases normal excitation versus the earlier proposal, but does not prove a valid LOW. Do not change pulls, bypass board components or increase voltage automatically if the result is unexpected.

The field supply must **not** connect to Pico GPIO/3V3/VSYS, carrier PWR1 or EVB USB-C. Carrier power remains Pico USB only; this adapter connects only to the verified isolated IN1 DC/COM pair.

## Approved constraints

- Keep EVB USB-C/PWR1, UEXT, all relay contacts and IN2..IN4 unwired.
- Disconnect Pico USB and the field source before attaching/repositioning wiring. Use only an enclosed supply's accessible low-voltage output; do not open a supply or probe/wire mains terminals.
- Identify polarity and terminals before connection; stop if labels/access are uncertain.
- No relay-ON operation, source changes, firmware/model/BSP/generated changes, flashing, installations or commits.
- Observe raw inputs using the existing output-only interface; do not seize/kill the user's terminal session.

## Staged procedure

1. **Source qualification (user-reported pass):** approximately +5 V measured directly across the disconnected adapter output. Its corrected rating is 5 V DC/0.5 A. No separate resistor qualification is required after the explicit amendment.
2. **Physical identification/preparation:** disconnect both supplies from the EVB. Identify IN1's actual DC/30VDC and COM positions against silkscreen/schematic; leave AC unused. Review before wiring, not by guessing from a rotated drawing.
3. **Cold field wiring:** adapter positive to IN1 DC, IN1 COM to adapter negative. Keep the source disconnected/off during connection/review; no system-GND jumper. Nothing attaches to Pico GPIO or carrier power inputs. Reconnect no source until instructed after the review.
4. **Inactive baseline:** power carrier with Pico USB only, IN1 excitation off. Check heartbeat, all relay indicators OFF, all four input readings HIGH. Measure GP2 against system GND, expected near the known 3.3 V rail.
5. **Excite IN1 only:** after preparation review, apply the qualified 5 V to IN1's field side. Observe IN1 indicator/raw state and other inputs/relay indicators. Measure GP2 against system GND; compare LOW and its voltage margin with authoritative RP2350 input/E9 limits before declaring a pass. Digital LOW alone is insufficient. Do not force a result by increasing voltage or modifying the circuit.
6. **Remove excitation:** disconnect field power. Confirm IN1 returns HIGH with near-3.3 V GP2, IN2..IN4 remain HIGH and relay indicators stay OFF. Record sample timestamps if available; manual switching does not establish 20 ms timing accuracy or complete pulse capture.
7. **Close pilot:** leave the field source disconnected and document measured values, source/wiring, observations and uncertainties. Additional channels or changed protection/excitation require further approval.

Unexpected relay activity/heating/supply behavior: remove power and investigate. Unexpected input voltage/logic: remove excitation and stop for review, not an automatic circuit/software workaround. Do not certify contact state, transient immunity or safety-critical operation from this pilot.

## Planned project changes

Task/index and relevant hardware/example documentation/results only. Raw logs/inspection artifacts remain under ignored project-local `build/`. No production source changes, dependency changes, installations, flashing or commits.

## Completion criteria

- Corrected source voltage/polarity and direct-drive limitation documented before excitation.
- Actual IN1 terminal identification and cold wiring reviewed before source attachment.
- Inactive/active/recovered raw states and GP2 voltages recorded, including A2/E9 margin limitations.
- Other inputs stay inactive, relay commands/indicators stay OFF, and contacts remain empty.
- Report pass/fail/uncertainty honestly; unexpected readings pause the pilot.
- Field source disconnected at completion; existing firmware and future test gates unchanged.

## Results and amendment history

The source was initially described as 12 V DC/1 A, with an external 2.2 kOhm resistor proposed and accepted as available. The user later reported 5 V, corrected the source to the only available 5 V supply, and confirmed the reading was directly at its output with no EVB connection. The user then explicitly rejected the external resistor and specified a 0.5 A supply rating. These statements supersede the initial 12 V/1 A and resistor assumptions. No field excitation or firmware/electrical modification was performed during this clarification.

The direct-input pilot follows the manufacturer's protected DC path; the supply's current rating must not be mistaken for precision/fault protection. The agent requested power-disconnected identification of IN1 DC/30VDC and COM before wiring. The user then reported that IN1 was already wired with correct polarity. This is user-reported wiring, not independent visual verification; actual source/carrier power state and whether excitation has occurred are not yet established. Do not record an inactive baseline or active test as passed.

The user confirmed the 5 V field supply is OFF. Wiring is user-reported, with no independent photograph/continuity check. The user then confirmed a fresh diagnostic snapshot showed IN1..IN4 all HIGH with Pico-USB-only carrier power and the field supply still OFF. This is user-observed terminal evidence, not an agent capture. With the field supply still OFF, the user measured GP2 against Pico system GND in DC voltage mode and reported **3.3 V**. The inactive baseline is therefore user-reported HIGH / 3.3 V. The user was instructed to enable the 5 V source only on the already wired IN1 DC/COM pair, retain Pico-USB-only carrier power, and disconnect on unexpected relay activation/heating. They pasted the diagnostic banner (revision 2/A2, E9 warning, relay commands OFF with no feedback) and these successive snapshots:

```text
t=29600ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
t=146760ms IN1=LOW IN2=HIGH IN3=HIGH IN4=HIGH
```

Only IN1 changed LOW, matching the intended active channel. These are user-pasted sampled records, not measured switching latency. With field excitation still ON, the user measured GP2 relative to Pico system GND in DC voltage mode and reported **0 V**. The nominal active condition therefore has agreeing digital LOW and approximately 0 V analog evidence on this A2 unit. Meter accuracy/resolution and transient/noise margins were not instrumented; this does not qualify other supplies/channels/temperatures or resolve E9 generally.

Following instructions to remove the 5 V field excitation while retaining Pico USB, the user pasted the same banner/history with this additional snapshot:

```text
t=301860ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
```

IN1 returned HIGH and the other inputs remained HIGH. The timestamps are sampled records of manual test stages, not switching latency. Asked to measure recovered GP2 against system GND with the field source OFF (expected near 3.3 V), the user answered yes. Record agreement with the approximate expected recovered level, not a newly supplied precise numeric measurement. The digital HIGH recovery is separately evidenced by the pasted snapshot.

The user confirmed all four relay indicators remained OFF throughout the pilot. Other input readings stayed HIGH in the pasted inactive/active/recovered records. The IN1 functional pilot therefore passed for the one nominal 5 V source and this A2 unit: inactive HIGH / 3.3 V; active LOW / approximately 0 V; recovered HIGH / user-confirmed near-3.3 V.

Functional HIGH -> LOW -> HIGH response is established under this one nominal condition only; no broader threshold/noise/timing qualification, proof of contact state or general A2/E9 resolution is implied. No firmware, pulls, resistor settings or other inputs were changed.

For final cleanup, the user was instructed to keep the field supply OFF, unplug Pico USB, and remove the IN1 field leads. They confirmed this was done: the board is user-reported unpowered and all terminals unwired again.

TASK-0023 is complete for the approved single-channel nominal 5 V pilot. No agent-controlled power action, serial read, firmware/model/BSP change, flash or commit occurred. IN2..IN4, relay actuation, broader voltage/tolerance/noise qualification and any further wiring remain separately gated.
