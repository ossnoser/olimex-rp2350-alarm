# LEG-5F coil power and proposed single-relay test

## Status and boundaries

TASK-0025 research/planning only. No firmware change, flashing, board power-on, relay energization, contact/load wiring or commit was performed. The user last confirmed the board unpowered with all terminals unwired after TASK-0024.

The installed baseline at the start of research had **no relay-ON interface**. Follow-up [TASK-0026](../tasks/TASK-0026-relay1-one-shot-implementation.md) implemented the REL1-only extension and later passed an approved user-observed empty-contact pulse/repeat-with-no-activation check. [TASK-0027](../tasks/TASK-0027-four-relay-sequence.md) implemented the requested R1..R4 sequence, maximum one coil ON, 1s ON/1s OFF gaps, then permanent within-boot lockout. That historical sequence build passed automated checks and an approved flash/startup verification, and an approved user-entered sequence produced the expected R1..R4/LOCKED software trace. The user confirmed the physical 1 -> 2 -> 3 -> 4 order and approximate one-second ON/OFF pattern; this is not contact/current/rail qualification. Its policy was one complete sequence per boot, not the earlier draft cooldown.

Current [TASK-0028](../tasks/TASK-0028-individual-relay-commands.md) replaces `testall` with `on N`, `off N` and global `off`, one relay maximum and **no automatic OFF or lockout**. Detected session loss/suspend still forces all OFF. This latched-control build passed automated checks and approved flash/startup, and an approved manual REL1 ON/selected-OFF trace (user-confirmed physical behavior, other relays OFF), not continuous-current/thermal validation. A silent host or CPU/undetected transport failure can leave a coil ON indefinitely. Prior brief pulses do not qualify continuous operation; powered tests remain separately gated, empty-contact and operator-present. The historical candidate below is not permission to energize a coil or connect a load.

## Identification and source quality

User transcription:

```text
recoy int'l leg 5f 15A 24V 5V DC | 15A 120V AC 50/60 Hz
```

This is consistent with the **Recoy/Rayex LEG-5F, 5 VDC coil** family. TME explicitly lists the manufacturer as Recoy/RAYEX ELECTRONICS and the exact part as LEG-5F, so the user's Recoy spelling need not be silently corrected to Rayex. The manufacturer-authored PDF carries the older RAYEX ELEC. branding.

Evidence:

1. [LEG-F SERIES manufacturer-authored PDF hosted by TME][leg-pdf], two pages, no visible revision/date. Page 1: ordering and coil table; page 2: general/mechanical data. Inspected downloaded text **and rendered table**, not search/AI-summary snippets.
2. [TME LEG-5F product listing][leg-product]: exact stock part, 5 VDC, SPDT, 70 Ohm, 72 mA, 360 mW. This corroborates the PDF but is distributor data, not proof of the user's individual coil.
3. [Leshan LMUN22xxLT1G manufacturer-authored series PDF hosted by iCEasy][driver-pdf], 16 pages: LMUN2235LT1G entries, input/bias resistors and maximum current. The inspected document has no relied-upon publication date.
4. [Olimex Rev. A schematic][evb-sch] and [manual][evb-manual] pinned to `5a4360986a678dc177df4ea6674fc5769e91e64d`: power path, drivers/indicator/flyback components, manual p. 9.
5. [Pico 2 datasheet][pico-datasheet], inspected Release 5, build 2026-07-03: typical current table and powerchain, sections 4.1 and 5.4/5.5.
6. Current project `bsp/common/usb_descriptors.cpp`, plus the installed SDK TinyUSB [`TUD_CONFIG_DESCRIPTOR` definition][tusb-descriptor] at commit `86ad6e56c1700e85f1c5678607a762cfe3aa2f47`.

Primary manufacturer websites could not be established from attempted Rayex/Recoy domains (DNS failures/parked page). Therefore provenance is **manufacturer-authored documents republished by distributors**, not a successful live manufacturer-site verification. Source variants/individual part authenticity remain limitations. Search snippets/AI overviews were navigation aids only, not technical evidence.

Artifacts remain in ignored `build/relay-research/`, including PDF/text/rendered pages and browser artifacts. No third-party PDF is copied into tracked documentation. Download SHA-256:

- LEG-F PDF: `78b1d7d57908ba8aa19ce3c7ab4c4e5e10bb05da1b940ecd137b38657e0850f5`
- LRC driver PDF: `fbd038062603f087818c9ae9b6a1c4a8b6ad63532986fe2253b5eae802238087`

The schematic still names **RAS-0515**; the manual photograph and user markings instead support LEG-5F. The user subsequently confirmed all four bodies have the same LEG-5F / 5 V DC marking with no extra M/L suffix. This supports the default-coil case, but neither a photograph nor coil-resistance/current measurements independently verify the individual parts.

## Coil data and calculated room-temperature current

For the **default 5 V / 0.36 W** LEG-F coil at 25 degrees C:

| Parameter | Source/interpretation |
|---|---|
| Nominal coil supply | 5 VDC |
| Coil resistance | 70 Ohm, +/-10% |
| Nominal current | Manufacturer lists 72 mA; 5/70 calculates 71.4 mA |
| Nominal power | 0.36 W |
| Maximum pickup voltage | 3.5 V at stated table conditions |
| Minimum dropout voltage | 0.5 V; not a guaranteed usable hold-voltage target |
| Operate/release | 8 ms maximum in general table; not a measured board timing guarantee |

At exactly 5 V and 25 degrees C, resistance tolerance alone gives approximately **65..79.4 mA** (77..63 Ohm). At the previously reported **4.9 V VSYS**, using zero path/driver drop as an upper-current illustration gives **70 mA nominal / 77.8 mA at 63 Ohm**. Actual coil voltage is lower by ferrite/wiring and driver drop. VSYS was measured with coils OFF, not during a pulse and not at the coil rail beyond L1.

Pickup must be evaluated from **voltage across the coil**, not only VSYS or the GPIO HIGH level. Do not call 3.5 V an all-temperature/all-variant target or 0.5 V a useful hold guarantee. Coil copper temperature and device tolerance matter; these are room-temperature planning estimates, not operating-envelope certification.

The ordering table distinguishes the default contact-rating `F` (15 A / 0.36 W) from additional coil-power suffixes. The 5 V row also publishes 55 Ohm / 90.9 mA and 31 Ohm / 161 mA alternatives. Their presence is important: **do not assume every LEG-F-family coil draws 72 mA**. The 0.64 W column has no published 5 V entry in the inspected table; do not invent one. Other table rows show apparent inconsistencies, so calculations here use only the coherent inspected 5 V row and corroborating exact-part distributor listing.

User-reported contact markings are not coil-current values. The part's contact family/variant ratings also do not certify carrier traces, terminals, insulation, load derating or mains suitability. All contacts remain empty.

## Carrier driver and power path

Source schematic/manual:

- GP10 -> REL1, GP11 -> REL2, GP12 -> REL3, GP13 -> REL4, all active-high.
- Each coil is low-side switched by an LMUN2235-family biased NPN. LRC data: internal 2.2 kOhm input-series / 47 kOhm base-emitter resistors, **100 mA collector absolute maximum at stated conditions**.
- Each relay has a green indicator with **2.2 kOhm** series resistor in parallel with its coil/driver path. The driver sinks both coil and indicator current.
- At 5 V, indicator current is less than 5/2200 = 2.3 mA, typically less after LED/driver drops. GPIO input/base-network demand is roughly a milliamp, supplied from the Pico's 3.3 V regulator; it is not coil current through a GPIO.
- The driver's saturation specification is at a much smaller test current (10 mA). Do **not** assume its quoted 0.25 V saturation maximum also holds at approximately 70..80 mA with 3.3 V drive.
- A default coil plus indicator is nominally below the 100 mA driver ceiling at room temperature, but that ceiling is not a recommended continuous design target or full-temperature approval. The higher-power alternatives can approach/exceed it, especially with tolerance. Do not energize an unidentified variant.
- A flyback diode is fitted across each coil. It suppresses inductive voltage but can lengthen physical release relative to an unclamped datasheet test. GPIO pulse duration is not guaranteed contact timing.

Pico-USB-only path:

```text
USB VBUS -> Pico D1 -> VSYS -> carrier EXT3.2 -> L1 -> carrier +5V coil rail
                        -> Pico regulator -> 3.3 V logic/base drive
```

TPS2116 external-input current ratings are not USB-current authorization or a measured current limiter for this reverse-fed coil path. Its input sources remain disconnected.

L1 is labeled `FB0805/600R/2A`: 600 Ohm denotes ferrite high-frequency impedance, **not a 600 Ohm DC series resistor**, and 2 A is not permission to draw 2 A from USB. Exact fitted part/DC resistance are unverified. C1 is marked NA/not assembled in the source; do not assume that output bulk capacitor is present or add one/jump power rails without approval.

DC coils are inductive loads; do not assume a capacitor-like current inrush multiplier. Nevertheless supply drop, switching/release transients, indicator/base current and host/cable/diode behavior need actual evaluation.

## Important USB budget finding: 250 mA, not 500 mA

The current descriptor passes **250** as the last argument to `TUD_CONFIG_DESCRIPTOR`. TinyUSB names that argument `_power_ma` and writes `_power_ma / 2` into `bMaxPower`.

Therefore this firmware advertises **250 mA configured bus power** (descriptor byte 125), **not 500 mA**. Inspection of the pre-extension diagnostic ELF's CONFIG at `0x10017538` confirms the nine-byte header `09 02 4b 00 02 01 00 80 7d`: `bMaxPower=0x7d=125` in 2 mA units. Prior macOS registry output's 250 allocation is consistent with this. A declaration is not a current measurement or a guarantee that a host/hub/cable can supply a particular transient. The user must also not confuse the separate 5 V / 0.5 A field adapter rating with the Pico USB budget; that adapter is disconnected for relay tests.

For the default coil:

- Incremental planning load is approximately **75..85 mA** for one channel including room-temperature tolerance/indicator and a small regulator-side base contribution; this is an estimate, not a measured maximum.
- Reserving a deliberately round **100 mA** for one default relay channel leaves **150 mA** of the declared budget for the Pico/carrier baseline and margin. This is an engineering allocation, not proof of baseline current or transient compliance.
- Pico 2's published `hello_usb` typical VBUS current is approximately 12.74 mA with external GPIOs unconnected; its datasheet explicitly says these numbers are **not guaranteed maxima**. Our QP image, board LEDs, drivers and actual hardware must not be equated to that example.
- One default coil is thus a plausible **conditional bench candidate**, subject to target identification, known USB source/cable and rail checks. The user identified the source as the USB-A port of an Apple USB-C/HDMI/USB-A/USB-C adapter powered only by the Mac, with no charging-port supply. Exact adapter/cable capability and current/voltage-under-load remain unmeasured; do not infer a 500/900 mA allowance.
- **Four nominal coils alone draw approximately 288 mA**, already exceeding the present 250 mA declaration before CPU/LED/driver consumption. Do not approve simultaneous four-coil operation on this profile from this calculation.

Do not energize before configured USB/explicit operator command; no automatic boot pulse. Proposed control must force OFF on detected USB/session loss or suspend and must not replay/resume a request after reconnect. Existing firmware has not added those relay-control paths. General USB suspend-current compliance is not established by this research.

## Physical preparation proposed for a later approved test

No powered probing is instructed by this report.

1. Confirm the chosen REL1's complete LEG-5F / 5 V marking and absence of a higher-power suffix; identify any different fitted parts. An optional unpowered coil-resistance consistency check is useful only with safely identified coil pads and all supplies removed. In-circuit parallel paths can affect readings; do not confuse contact terminals with coil pins or desolder parts for this task.
2. Identify safe system GND and carrier +5V coil-rail points beyond L1. Positive coil pad/flyback-diode cathode are schematic-connected options, but their actual physical identity/access must be reviewed. Do not guess pads, probe under a mounted module or bridge tiny components. PWR1 is a mux **input**, not a substitute coil-rail measurement point.
3. If access is poor, stop and arrange a safely insulated measurement fixture while unpowered under separate approval. Do not improvise powered soldering or change the circuit.
4. With later power approval, compare unloaded VSYS and the actual coil rail. Then use an approved pulse to check loaded behavior, USB continuity, indicators/other GPIO observations. A slow multimeter may miss a one-second minimum or brief transients; no unsafe timeout extension to obtain a reading.
5. Inline USB current instrumentation, if available and data-capable, can improve the budget evidence. Do not put a current-mode meter across rails or cut/rewire a USB cable under this plan. Without a current measurement, retain the explicit unmeasured-budget limitation.

## Historical candidate behavior - superseded by agreed one-shot policy

This was the research draft. The first agreed implementation was TASK-0026: `test1`, one-second REL1 pulse once per boot, `off`, session-loss cancellation, no input linkage. It is preserved as historical evidence/rollback. Current TASK-0027 replaces `test1` with `testall`, running all four sequentially in seven nominal seconds and retaining `off`/session-loss cancellation during pulses and gaps. In particular the draft five-second cooldown below was rejected in favor of permanent within-boot lockout. Only the prior REL1-only pulse/negative repeat were approved/observed; the new four-relay image has since passed approved flash/startup, and its first separately approved manual sequence produced the expected commanded-output trace. Physical order and approximate one-second ON/OFF pattern were user-confirmed; further actuation still requires separate permission.

Original draft, retained for traceability:

- **REL1 only**, all other outputs remain OFF; no field input/automatic control and no contact wiring.
- Explicit fresh USB request, with a proposed **one-second software pulse**, then automatic OFF. Never redirect the heartbeat's 1 Hz duty cycle to a mechanical relay.
- Reject requests while active and during a proposed **five-second cooldown**. A new request must not extend/restart a running pulse; buffered commands must not produce an automatic train of later pulses.
- An explicit STOP/OFF action always wins; detected framework/init faults still clear all outputs before halt. No automatic reset policy change without approval.
- Reject requests without an active configured terminal/session; detected disconnect/suspend cancels the pulse, clears pending requests and requires a fresh explicit request later. No automatic resume.
- Keep the onboard LED heartbeat independent and preserve model/BSP separation. Candidate architectural approach: explicit relay QHsm owned by the independently copied Blinky AO, with its own timeout, while DiagnosticAO handles bounded commands/reporting. The exact design/protocol is still a question, not an implementation instruction.
- Fixed queues/buffers and nonblocking logging. Report commands/state, never fictitious contact feedback. TX backpressure must not postpone OFF or create a queued actuation sequence.
- Start/reboot/fault OFF; tests for timeout, repeated requests, cooldown, STOP, USB loss, malformed input and existing heartbeat/input/USB regressions before any flash/test approval.

A software timeout is bounded only while firmware/timer/scheduler function correctly. A total CPU failure can leave a GPIO latched until power removal; this is **not a hardware-enforced maximum ON time**. With all contacts empty and an operator present, this limitation must be explicit. A later alarm/load controller needs a separate fail-safe/interlock/watchdog design; do not silently add auto-reset or claim fail-safe protection here.

## Remaining individual decisions/gates

1. Fitted markings/suffix consistency are user-confirmed; source is the Mac-powered Apple adapter. Safe rail access, coil/board current and loaded behavior remain unverified.
2. REL1-only, nominal one-second pulse, one-per-boot lockout, OFF and session-loss policy were agreed. No repeatable cooldown is used.
3. Software implementation/testing was authorized by the user's instruction to proceed and is recorded under TASK-0026. TASK-0025 itself remains a research record.
4. Separately approve flashing, preparation/rail checks and first unloaded actuation. Keep field sources/modules/contacts disconnected. Stop on unexpected activation/heating/re-enumeration/supply behavior; do not retry automatically.

No part of the report establishes load/mains/contact ratings or authorizes connecting a real load.

[leg-pdf]: https://www.tme.eu/Document/facc22af0219a1d2944a33df4662b473/LEG-F-SERIES.pdf
[leg-product]: https://www.tme.eu/si/en/details/leg-5f/miniature-electromagnetic-relays/recoy-rayex-electronics/
[driver-pdf]: https://img.iceasy.com/product/product/files/202108/00b81e7e3d4b84a4e1e90e3adf745e40cafc53fd.pdf
[evb-sch]: https://github.com/OLIMEX/PICO-EVB/blob/5a4360986a678dc177df4ea6674fc5769e91e64d/HARDWARE/PICO-EVB_Rev_A/PICO-EVB_Rev_A.pdf
[evb-manual]: https://github.com/OLIMEX/PICO-EVB/blob/5a4360986a678dc177df4ea6674fc5769e91e64d/DOCUMENTS/PICO-EVB-user-manual.pdf
[pico-datasheet]: https://datasheets.raspberrypi.com/pico/pico-2-datasheet.pdf
[tusb-descriptor]: https://github.com/hathach/tinyusb/blob/86ad6e56c1700e85f1c5678607a762cfe3aa2f47/src/device/usbd.h#L221-L222
