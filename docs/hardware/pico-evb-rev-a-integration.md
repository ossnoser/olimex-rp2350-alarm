# Pico 2 on Olimex PICO-EVB Rev. A

## Status and scope

TASK-0021 is a **schematic/manual investigation**, not a powered-board validation or firmware implementation. The user identified a non-wireless Pico 2 / RP2350-PICO2-HDR and PICO-EVB Rev. A, approved unloaded low-voltage investigation, and proposed Pico USB as the sole supply. Relay contacts, input terminals and external modules remain disconnected. No power-on, flashing, source-code change or commit is authorized by this research task.

The conclusions below assume an unmodified, correctly assembled Rev. A carrier and a standard 40-pin Pico 2 module. Verify physical orientation, markings and assembly before using them as wiring instructions. Continue using `PICO_BOARD=pico2`; a carrier-specific initialization layer is a future firmware decision, not a different RP2350 architecture.

Follow-up: [TASK-0022](../tasks/TASK-0022-pico-evb-diagnostic-plan.md) adds an independent [relay-off diagnostic example](../../examples/pico-evb-diagnostic/README.md) and carrier BSP. Its automated validation does not replace the physical checks/gates below; existing Blinky examples still do not initialize relays.

## Primary sources and revision control

Olimex PICO-EVB sources were inspected at commit **`5a4360986a678dc177df4ea6674fc5769e91e64d`**:

- [Rev. A schematic PDF][evb-schematic] and [KiCad source][evb-kicad], sheet 1, dated **2026-05-15**.
- [PICO-EVB manual][evb-manual], Rev. 1.0, July 2026: layout pp. 6-7, inputs p. 8, relays p. 9, power p. 10, UEXT p. 12.
- [Arduino demo][evb-demo], corroborating GPIO lists and active levels; it is not the proposed safe-first firmware.
- [Pico 2 datasheet][pico-datasheet], sections 3.1, 5.4 and 5.5, for header pins and USB/VSYS power.
- [TPS2116 datasheet][mux-datasheet], features and device functional modes, for mux priority/reverse-current blocking.
- [RP2350 datasheet][rp-datasheet], hardware revision history and RP2350-E9, for revision-dependent input leakage.
- [MOD-RS485 manual][rs485-manual], initial April 2011, pp. 5-6, and [Rev. A schematic][rs485-schematic], 2013, for future UEXT direction-control mapping. Actual module revision/jumpers remain to be verified.

Downloaded PDFs, extracted text, source files and rendered inspection images are kept under ignored `build/pico-evb-research/`, not redistributed in this report. This report records derived facts and original integration analysis, not copied manufacturer diagrams. Third-party licenses/notices remain with their sources.

Local SHA-256 records for the two key immutable PDFs:

- Schematic: `6a2a29758cf7ec70673fdaac13d4486e27bd1d9aae0d26d3d6e01f1ec4e4d8c7`
- Manual: `a85a6f99a2a3eb3d0cddf3bd3f342db3701343cf4f4e46b68598a367948a210e`

These identify the inspected downloads; they are not independent certification of the physical board.

## Confirmed Rev. A input and relay mapping

Pico physical pin numbers below refer to the standard 40-pin module, **not GPIO numbers**. On the left carrier socket EXT2, connector pin numbering matches Pico physical pins 1..20.

| Generic resource | Carrier signal | RP2350 GPIO | Pico physical pin / EVB connection | Active level |
|---|---|---|---|---|
| input1 | IN1 / OPT1 collector | GP2 | 4 / EXT2.4 | LOW = energized input |
| input2 | IN2 / OPT2 collector | GP3 | 5 / EXT2.5 | LOW = energized input |
| input3 | IN3 / OPT3 collector | GP8 | 11 / EXT2.11 | LOW = energized input |
| input4 | IN4 / OPT4 collector | GP9 | 12 / EXT2.12 | LOW = energized input |
| output1 | REL1 / T1 | GP10 | 14 / EXT2.14 | HIGH = relay energized |
| output2 | REL2 / T2 | GP11 | 15 / EXT2.15 | HIGH = relay energized |
| output3 | REL3 / T3 | GP12 | 16 / EXT2.16 | HIGH = relay energized |
| output4 | REL4 / T4 | GP13 | 17 / EXT2.17 | HIGH = relay energized |

The schematic and manual circuit diagrams agree with the upstream demo's input `{2,3,8,9}` and relay `{10,11,12,13}` lists. This establishes routing, not alarm-zone meanings, debounce requirements or load safety.

### Inputs

Each field-side input connector IN1..IN4 has:

| Terminal number in schematic | Label/function |
|---|---|
| 1 | High-voltage AC input, outside this investigation's permitted testing |
| 2 | Low-voltage DC input, positive relative to that channel's COM |
| 3 | COM, that channel's field-side return |

Do not infer the physical terminal order from a rotated photograph: match the actual board silkscreen. Never connect the DC and AC terminals of the same channel simultaneously.

The vendor specifies 3.3..30 VDC for the DC path. Its 3.9 kOhm series resistor feeds the input indicator LED and PC817-family optocoupler LED. The AC path instead includes two 150 kOhm resistors and reverse-protection diodes; no mains connection procedure is provided here.

On the MCU side, R19..R22 are **47 kOhm pull-ups to 3.3 V**, and each optocoupler emitter is grounded. Therefore:

- With the field terminal disconnected, a working/powered channel is expected to read HIGH (inactive).
- With sufficient correctly polarized field input, the optocoupler pulls the GPIO LOW (active).
- This is voltage sensing, not a self-powered dry-contact input. A bare switch without an excitation supply does not create an input signal.
- Each field COM is separate in the schematic, not a direct connection to Pico GND or another channel's COM. Deliberately sharing a supply/return can remove that isolation advantage.
- Start future firmware from the known external pull-up circuit rather than blindly copying the demo's `INPUT_PULLUP`. Any extra internal pull affects required optocoupler sink current and must be validated, especially at the 3.3 V lower limit.
- Binary inactive and a broken/disconnected field wire look alike; tamper/EOL supervision requires a separate requirements/circuit decision.

AC-driven channels can produce pulses through the mains cycle. The demo's 5 ms activation / 60 ms deactivation delays are demonstration choices, not validated alarm filtering or timing requirements. Only low-voltage DC testing is in the proposed development path.

### Relays

Each GP10..GP13 drives an LMUN2235-family biased NPN transistor, which sinks a nominal 5 V relay coil. A flyback diode and green status LED are present. The driver symbol includes 2.2 kOhm base-series and 47 kOhm base/emitter bias resistors; the latter assists default-off behavior but is not a safety interlock.

Logical control is **active-high**, not active-low. Inactive output is LOW. Contact blocks CON_REL1..4 are drawn as:

| Terminal | Contact |
|---|---|
| 1 | NO |
| 2 | COM |
| 3 | NC |

These are separate from input COM terminals. With a de-energized relay, COM-NC remains connected: "relay off" does not mean every load circuit is open. Leave all contact terminals unwired for initial tests.

The schematic names RAS-0515, while the manual's board photograph shows LEG-5F markings. Confirm the actual fitted relay and its datasheet before budgeting coil current or deciding load ratings. Also, the older project baseline's `15 V / 240 VAC` entry conflicts with the vendor's `15 A / 240 VAC` marketing description: that is a documentation-unit discrepancy, not a validated board-level operating limit. PCB traces, terminals, clearance, load type, derating and actual relay markings require their own review. No contact-current/mains capability is established here.

## Power arrangement

### Proposed Pico-USB-only source is documented and schematic-consistent

The manual explicitly lists Pico USB, carrier USB-C and the 5 V terminal as supported supply sources. With only Pico USB connected, the relevant path is:

```text
Mac/data-capable USB cable -> Pico VBUS
                          -> Pico's D1 Schottky -> Pico VSYS
                          -> EVB EXT3.2 / VDD_SYS -> ferrite L1 -> EVB +5V coil rail

Pico VSYS -> Pico's 3.3 V regulator -> EXT3.5 -> input pull-ups / UEXT +3.3 V
```

EXT3.1 is the Pico physical VBUS pin 40; EXT3.2 is VSYS pin 39, and EXT3.5 is the 3.3 V output pin 36. The `+5V` carrier rail can be below exactly 5 V when supplied through Pico D1 and wiring losses. Validate the available voltage/current before energizing coils; no simultaneous-four-coil USB current budget has been established.

In the other direction, EVB external inputs feed TPS2116 U1, whose output supplies the carrier +5V rail and Pico VSYS through L1. Carrier USB-C is VIN1; PWR1 is VIN2. MODE and the PR1 divider select USB-C priority, matching manual p. 10. TPS2116 reverse-current blocking and Pico D1 explain the intended separation between sources. The manual permits simultaneous supplies, but that is not a blanket guarantee for incorrect voltage, polarity, damaged hardware or externally driven I/O.

For this investigation retain the approved conservative arrangement:

- **Only the Pico USB connector** supplies power/data.
- EVB USB-PWR1 and PWR1 remain disconnected.
- No power-rail shorting, solder changes or added bypass jumpers.
- PWR1 is marked **5.0 VDC only**, not 12/24 V. Its shunt TVS is not a reverse-polarity series blocker.
- The carrier USB-C connector's D+/D- pins are unconnected: it is **power only**, not USB programming or CDC serial. Use Pico USB for BOOTSEL/serial.
- The Rev. A schematic has no power-selection configuration jumpers. EXT1 and C1 are marked not assembled; they are not user power-selection jumpers.
- Keep external modules and field wiring disconnected, avoiding unintended power paths through UEXT or I/O.

This verifies the proposed topology on paper, not cable current capacity, actual rail voltage or as-built assembly.

## Critical EXT1/Pico-versus-PICO30 distinction

The schematic explicitly warns that EXT1 labels **GPIO23, GPIO24, GPIO25 and GPIO29 are only extra GPIO with Olimex RP2040-PICO30**. With a standard Raspberry Pi Pico/Pico 2 on EXT2/EXT3, those connector positions are **GND**:

| EXT1 pin | Label on schematic | With standard Pico 2 |
|---|---|---|
| 5 | GPIO29 / GND | GND |
| 9 | GPIO25 / GND | GND |
| 10 | GPIO24 / GND | GND |
| 11 | GPIO23 / GND | GND |

Do not drive these header positions or attach an external source expecting a GPIO. They are not the module's internal GP23/24/25/29. In particular, existing onboard-LED firmware using **internal GP25 remains valid**, but EXT1's GPIO25 label is not a way to access that LED signal.

EXT1's normally available Pico GPIO labels are GP14, GP15, GP18..GP22 and GP26..GP28. RUN, 3V3_EN and ADC_VREF are special signals, not spare GPIO. EXT1 is marked `NA(HN1x18)` in the schematic; whether a header is fitted on the user's board is unverified.

## Startup/reset/error implications

The existing Blinky BSP configures the onboard LED and USB only. It **does not explicitly initialize carrier relays**. Do not equate the standalone smoke tests with verified carrier-safe startup.

For a later approved carrier BSP/example:

1. Preload GP10..GP13 output latches LOW before enabling their output drivers, as early as practicable in startup.
2. Keep all four LOW unless a separately approved command/test explicitly requests energization.
3. Initialize GP2/3/8/9 as inputs with a reviewed pull policy and expose active-low readings through generic BSP resources.
4. Evaluate reset, BOOTSEL, watchdog, power-up/down, debugger attachment and faults separately. GPIO/high-impedance behavior outside application control must be checked on the actual assembly.
5. The current QP fail-stop handler halts but does not force future relay outputs off. A carrier fault/shutdown policy needs explicit design; a halted CPU can leave an energized output latched HIGH.
6. Do not redirect the one-second LED duty-cycle example onto a mechanical relay. Do not adopt the upstream demo's automatic input-to-relay activation as the first test.

### RP2350 silicon-revision caveat

RP2350-E9 affects **A2** GPIO input pads and can source current in the undefined voltage region, defeating weak pull-downs; it is fixed in **A3** (and retained in A4). The current datasheet notes that reset pull-downs work immediately after POR/RUN reset because input-enable is initially clear, so E9 is not evidence that every reset will energize relays. It is still relevant to later input/high-impedance configuration and weak driver biasing.

Do not assume board label "Pico 2" identifies silicon stepping. A later diagnostic can report `rp2350_chip_version()` (revision 2 versus 3 distinguishes A2 from A3/A4 for this issue). Validate actual optocoupler LOW/HIGH levels, especially at minimum DC excitation, and do not make A2-specific GPIO/resistor modifications without an explicit hardware/firmware review. The 47 kOhm input pull-ups and NPN bias network alone do not constitute an input-threshold or safety proof.

## UEXT and future MOD-RS485

| UEXT pin | Carrier route | Pico physical pin | Function |
|---|---|---|---|
| 1 | +3.3 V | 36 | Module power from Pico regulator |
| 2 | GND | Ground | Logic return |
| 3 | GP0 | 1 | UART0 TX |
| 4 | GP1 | 2 | UART0 RX |
| 5 | GP17 | 22 | I2C0 SCL; 2.2 kOhm pull-up |
| 6 | GP16 | 21 | I2C0 SDA; 2.2 kOhm pull-up |
| 7 | GP4 | 6 | SPI0 RX/MISO |
| 8 | GP7 | 10 | SPI0 TX/MOSI |
| 9 | GP6 | 9 | SPI0 SCK |
| 10 | GP5 | 7 | SPI0 CSn |

UEXT is 3.3 V logic, not a true bipolar RS232 interface. USB CDC does not consume GP0/GP1, and UART stdio is disabled in the present project.

The cited MOD-RS485 documents indicate:

- Host UART0 TX GP0 -> UEXT.3 -> transceiver DI; RO -> UEXT.4 -> GP1 RX.
- Default SCL/SCK jumper at **SCK**: DE uses UEXT.9 -> **GP6**, HIGH enables the transmitter.
- Default #SS/SDA jumper at **#SS**: /RE uses UEXT.10 -> **GP5**, LOW enables reception.
- Alternate selections move DE to GP17 and /RE to GP16. This conflicts with I2C and encounters the carrier's 2.2 kOhm pull-ups; in particular DE can be pulled HIGH before firmware, unintentionally enabling transmission.
- Default direction controls conflict with using UEXT SPI clock/chip-select for another peripheral. They do not overlap the four input or relay GPIO groups.
- Do not assume an I2C device or tie DE and /RE together without reviewing the actual module revision, solder-jumper positions and optional components.
- The module manual defaults termination enabled. A multi-node bus needs its own termination/bias/reference/isolation review; this report does not authorize RS485 wiring.

## Proposed first-test sequence: gated, not executed

1. **Power off and disconnect everything.** Verify the Rev. A silkscreen, correct module/header orientation and no offset rows/bent pins. Compare USB-end orientation with the manual's module outline/USB1 mark; do not rely solely on a guessed rotation. Leave input/contact terminals, MOD-RS485/UEXT modules and EVB power inputs unwired.
2. **Obtain a separate firmware-task approval.** First implement an early relay-off carrier initialization and a USB diagnostic reporting silicon revision and raw input states, with no automatic input-to-relay actions and no relay energization commands initially. Program/verify the Pico standalone before mounting where practicable.
3. **After separate power/test authorization**, assemble unpowered, then use only the Pico data USB connection. Verify power/serial, all four relay indicators off, and unloaded inputs expected inactive/HIGH. If an unintended relay energizes or supply behavior is abnormal, disconnect power and investigate; do not attach loads.
4. Check relay-off behavior across startup, reset/BOOTSEL and terminal reconnect before expanding tests. Record observations; a dark LED is not a certified contact-state or safety measurement.
5. **Later, separately approve input excitation** with a current-limited, known low-voltage DC source and verified terminal polarity/numbering. Test one channel at a time, including raw GPIO levels and debounce needs. Do not introduce mains testing or infer that a bare switch is sufficient.
6. **Later, separately approve relay actuation** one channel at a time with contact blocks still empty. Confirm coil supply/current and actual relay markings first. Contact/load wiring, especially mains, requires a separate qualified electrical/safety assessment.

No physical test was performed in TASK-0021. Open items before later stages are actual assembly/orientation, relay part/current budget, Pico silicon stepping, raw input voltage margins, reset/fault behavior, sensor/debounce requirements and the future RS485 module's actual jumper configuration.

[evb-schematic]: https://github.com/OLIMEX/PICO-EVB/blob/5a4360986a678dc177df4ea6674fc5769e91e64d/HARDWARE/PICO-EVB_Rev_A/PICO-EVB_Rev_A.pdf
[evb-kicad]: https://github.com/OLIMEX/PICO-EVB/blob/5a4360986a678dc177df4ea6674fc5769e91e64d/HARDWARE/PICO-EVB_Rev_A/PICO-EVB_Rev_A.kicad_sch
[evb-manual]: https://github.com/OLIMEX/PICO-EVB/blob/5a4360986a678dc177df4ea6674fc5769e91e64d/DOCUMENTS/PICO-EVB-user-manual.pdf
[evb-demo]: https://github.com/OLIMEX/PICO-EVB/blob/5a4360986a678dc177df4ea6674fc5769e91e64d/SOFTWARE/Arduino-demo/pico-evb.ino
[pico-datasheet]: https://datasheets.raspberrypi.com/pico/pico-2-datasheet.pdf
[mux-datasheet]: https://www.ti.com/lit/ds/symlink/tps2116.pdf
[rp-datasheet]: https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
[rs485-manual]: https://www.olimex.com/Products/Modules/Interface/MOD-RS485/resources/MOD-RS485.pdf
[rs485-schematic]: https://www.olimex.com/Products/Modules/Interface/MOD-RS485/resources/MOD-RS485-schematic.pdf
