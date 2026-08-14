# Hardware open points

This document lists hardware information that must be collected or clarified before final design and firmware implementation decisions are made.

## Board and target details

The active firmware target is confirmed as `RP2350-PICO2-HDR`, always mounted on Olimex `PICO-EVB`.

Remaining points:

- Collect the official `RP2350-PICO2-HDR` product page, schematic, pinout, and datasheet links.
- Confirm the exact pin mapping between `RP2350-PICO2-HDR`, `PICO-EVB`, and firmware GPIO names.
- Confirm the C/C++ SDK and board definition support for `RP2350-PICO2-HDR`.
- Confirm boot, flashing, and debug procedures for the selected board.

## Alarm node requirements

- Number of identical RS485 nodes required in the complete system.
- Required inputs and outputs per node.
- Required global inputs and outputs outside individual nodes, if any.

## Input requirements

The firmware resources are currently named generically as `input1` ... `input4`.

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
- Exact GPIO mapping for UART TX/RX and `DE` / `/RE` control signals.

## Power supply

The firmware project assumes node power is provided as `5 V`.

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

- Final pin assignment.
- Relay control polarity.
- Input read polarity.
- UEXT UART selection.
- RS485 driver-enable and receiver-enable GPIO mapping.
- Boot and programming method.
- Debug interface availability.
- Watchdog and safe-state requirements.
