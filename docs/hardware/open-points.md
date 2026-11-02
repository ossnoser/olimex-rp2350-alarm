# Hardware open points

This document lists hardware information that must be collected or clarified before final design and firmware implementation decisions are made.

## Board and target details

The active firmware target is confirmed as `RP2350-PICO2-HDR`, always mounted on Olimex `PICO-EVB`.

Known references and software baseline:

- Official product/Pico 2 references are collected in [development references](../development/references.md).
- `PICO_BOARD=pico2` support is build/standalone-tested; the user confirmed that the HDR variant uses the same software target. See [board support](../development/board-support.md).
- Rev. A input/relay/UEXT routing, active levels and power paths are documented from primary sources in [the carrier integration investigation](pico-evb-rev-a-integration.md). This is schematic research, not powered carrier validation.

Remaining points:

- Verify actual assembly/orientation and fitted relay part. TASK-0022 standalone USB reporting identified the tested module as raw revision 2/A2 (GPIO E9 applies); identify other modules separately.
- Validate active-input voltage margins, BOOTSEL/watchdog/debugger/fault transients, coil rail beyond L1 and available load current on the physical carrier.
- TASK-0022 implemented carrier-specific initialization and passed automated plus initial unloaded smoke checks: user-reported 3.3 V logic supply, 4.9 V VSYS, approximately 3.27 V inactive inputs and corrected 0 V relay GPIO readings; raw inputs HIGH and normal power-cycle behavior matched expectations. These are limited observations, not electrical/load safety certification. Further excitation/actuation/debug tests need separate approval; see [the diagnostic README](../../examples/pico-evb-diagnostic/README.md).

## Alarm node requirements

- Number of identical RS485 nodes required in the complete system.
- Required inputs and outputs per node.
- Required global inputs and outputs outside individual nodes, if any.

## Input requirements

The firmware resources are currently named generically as `input1` ... `input4`.

Rev. A maps these to GP2/3/8/9, active-low with external 47 kOhm pull-ups. Minimum-excitation margins and pull policy still require physical validation. The TASK-0022 standalone test identified this module as A2, so E9 applies. Its standalone raw readings, without EVB pull-ups, were not carrier validation; later unloaded mounted-board GPIOs read HIGH and approximately 3.27 V. [TASK-0023](../tasks/TASK-0023-in1-low-voltage-input-test.md) subsequently passed a user-operated IN1-only nominal 5 V pilot: HIGH/3.3 V -> LOW/approximately 0 V -> HIGH/near-3.3 V, other inputs HIGH and relay indicators OFF. Source current rating was 0.5 A capacity; only the onboard DC resistor was used. The field source/wires were removed and the board left unpowered. [TASK-0024](../tasks/TASK-0024-remaining-input-validation.md) then recorded user-supplied HIGH -> LOW -> HIGH transitions for each channel, with others HIGH and all relay indicators OFF; cleanup to an unpowered/unwired board was confirmed. IN2..IN4 active analog GPIO voltages were explicitly not measured. Digital functional mapping passed, but their analog margins, minimum-excitation/tolerance/noise behavior and general E9 robustness remain unvalidated.

Clarify the required electrical type and behavior for each input resource when assigned to a real function:

- contact lines;
- inertial lines;
- infrared sensors;
- gas sensors;
- smoke sensors;
- battery status;
- tamper lines;
- any existing wired alarm loops.

For each input type, collect:

- normal voltage/current levels;
- active state;
- fault/tamper detection requirements;
- debounce/filtering requirements;
- end-of-line resistor requirements, if any;
- isolation requirements.

## Output requirements

The firmware resources are currently named generically as `output1` ... `output4`.

Rev. A relay control routes are GP10..GP13, active-high; this resolves schematic polarity, not safe load behavior. The source schematic and manual photograph name different relay models, so fitted markings/current limits must be checked. The earlier baseline's `15 V / 240 VAC` wording also conflicts with the vendor's `15 A / 240 VAC` marketing description; correct/qualify that documentation before using any load rating. No board-level load capability is certified by this research.

TASK-0025 user markings support Recoy/Rayex LEG-5F / 5 VDC. Distributor-hosted manufacturer data specify the default 70 Ohm +/-10%, approximately 72 mA / 0.36 W coil at 25 degrees C; higher-power variants exist. [Relay power planning](relay-power-and-single-test-plan.md) also confirms the current USB descriptor declares **250 mA**, so four nominal coils alone exceed that allocation. The user confirmed matching LEG-5F / 5 VDC markings without M/L on all four bodies. Actual coil rail/current and loaded/transient behavior remain quantitatively unverified. TASK-0026 subsequently passed an approved user-observed REL1-only pulse and repeat-with-no-activation check; this does not qualify contacts/current/rail margins. [TASK-0027](../tasks/TASK-0027-four-relay-sequence.md) implements sequential R1..R4 with only one coil commanded at a time and within-boot lockout; its approved flash/startup and one unloaded sequence subsequently passed, with user-confirmed physical order/approximate ON/OFF pattern and final power-off cleanup. This is not loaded rail/current/contact/transient qualification; further actuation remains separately gated. No actuation is authorized by the original research alone.

Clarify the required electrical type, load, and safety constraints for each output resource when assigned to a real function:

- LEDs;
- relays;
- sirens;
- lights;
- gate controls;
- door controls;
- gas shutoff;
- boiler controls;
- other actuators.

For each output type, collect:

- voltage and current;
- normally-open/normally-closed behavior;
- required isolation;
- fail-safe behavior;
- maximum switching frequency;
- protection requirements.

## RS485 network

`MOD-RS485` is part of the mandatory node hardware baseline.

Remaining points:

- Required RS485 topology.
- Required cable type and maximum cable length.
- Required termination and biasing strategy.
- Required protocol compatibility with existing nodes or future nodes.
- Required baud rate.
- Required addressing model.
- Required behavior during bus faults.
- Required galvanic isolation, surge protection, and ESD protection.
- Verify the actual module revision/jumpers before applying the documented default mapping: UART0 GP0/GP1, DE on GP6 (UEXT SCK) and /RE on GP5 (UEXT CSn). Alternate control positions use pulled-up GP17/GP16 and conflict with I2C; review startup bus-drive risk and peripheral sharing.

## Power supply

The firmware project assumes node power is provided as `5 V`.

Rev. A documentation supports the proposed Pico-USB-only supply, with EVB USB-C and PWR1 left disconnected. Pico VSYS feeds the carrier coil rail through L1 and can be below USB VBUS because of the module's Schottky diode. Carrier USB-C is power-only. No power-on or current-budget test was performed in TASK-0021.

Remaining points from the firmware perspective:

- Confirm whether firmware must monitor supply status.
- Confirm whether brown-out behavior must be handled explicitly.
- Confirm whether any power-good or battery-status signal is connected to a firmware input.
- Confirm safe output states during startup, reset, and power loss.

## Mechanical and installation constraints

- Required enclosure.
- DIN rail or wall-mount requirements.
- Terminal layout.
- Cable access and strain relief.
- Serviceability requirements.
- Environmental constraints.

## Firmware-related hardware decisions

- Final application-resource meanings and carrier BSP implementation using the documented Rev. A mapping.
- TASK-0022 implements early relay-off and a registered OFF-before-halt fault action for the diagnostic only. Verify actual reset/boot/power-loss behavior and define final alarm-node shutdown requirements.
- Input pull/filter policy and voltage margins for the actual silicon/field source.
- Reserve UART0/UEXT resources and verify MOD-RS485 control jumpers before enabling that module.
- Mounted-board boot/programming procedure and physical debug access.
- Watchdog, interlocks and application-specific safe-state requirements.
