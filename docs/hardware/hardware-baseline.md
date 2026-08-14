# Hardware baseline

## Purpose

This document records the active hardware baseline for the embedded alarm node firmware project.

## Related decision

The target board decision is documented in:

- [DEC-0001 - RP2350-PICO2-HDR target board](../decisions/DEC-0001-rp2350-pico2-hdr-target.md)

## Node hardware baseline

Each alarm node is composed of the following mandatory hardware components:

- `RP2350-PICO2-HDR` target board;
- Olimex `PICO-EVB` carrier board;
- Olimex `MOD-RS485` communication module.

Multiple identical nodes are connected through an RS485 bus.

## Firmware-visible power model

The firmware project considers each node as powered from `5 V`.

Any upstream system-level power architecture, including conversion from other voltages to `5 V`, is outside the scope of this firmware project unless explicitly documented by a future task.

## RP2350-PICO2-HDR target board

The firmware target board is `RP2350-PICO2-HDR`.

The `-HDR` suffix identifies the version with soldered pin headers. These headers allow the board to be inserted into the Olimex `PICO-EVB` socket.

In this project, the board is always used mounted on the Olimex `PICO-EVB` carrier board.

## Olimex PICO-EVB carrier board

Source description: evaluation board for Raspberry Pi Pico-compatible boards with four optoisolated inputs, four relays, UEXT connector, and USB-C power supply.

Relevant recorded features:

- Raspberry Pi Pico-compatible 40-pin connector.
- Four relay outputs rated by the source page as `15 V / 240 VAC`.
- Relay contacts are Form 1C with normally closed and normally open contacts.
- Relay signals are available on screw terminals.
- Four relay status LEDs.
- Four optoisolated inputs.
- Input operating ranges listed by the source page:
  - `3.3-30 VDC`;
  - `110-220 VAC`.
- Four input status LEDs.
- External power supply options listed by the source page:
  - screw connector;
  - USB-C;
  - Pico module.
- UEXT connector with `I2C`, `UART`, and `SPI` signals.
- Board dimensions: `100 x 70 mm`.

## Firmware generic I/O resources

The `PICO-EVB` inputs and outputs are treated as generic firmware resources at this stage.

Inputs:

- `input1`;
- `input2`;
- `input3`;
- `input4`.

Outputs:

- `output1`;
- `output2`;
- `output3`;
- `output4`.

No final alarm-zone meaning, sensor type, actuator type, or safety behavior is assigned to these resources in the baseline.

## Olimex MOD-RS485

Source description: `RS485/RS422 converter module with UEXT`.

The `MOD-RS485` module is part of the mandatory node baseline hardware.

Relevant recorded features:

- Intended to connect development boards with UEXT connector to RS485 interface networks.
- Uses `ADM3483ARZ` low-power differential line transceiver.
- Operates from a single `3.3 V` power supply.
- Half-duplex communication.
- Comes with a `10 pin` UEXT cable.
- Data rate up to `250 kbps` according to the source page.
- Uses UART interface and two control pins.
- Uses UEXT connector and RS485 connector.
- Dimensions: `30.99 x 20.32 mm` / `1.22 x 0.80 in`.
- UEXT UART signals are on pins `#3` and `#4` according to the source FAQ.
- Default control signals use pins `9` and `10` according to the source FAQ.
- Control signals correspond to `ADM3483ARZ` signals `DE` and `/RE`.
- The module does not use I2C despite possible reuse of pins marked `SCL` and `SDA` as digital I/O depending on jumper positions.

## Previous-generation board clarification

`RP2040-PICO` is the previous-generation Pico board family and is not part of the active hardware baseline for this project.

The firmware, pin mapping, build configuration, and board support decisions must target `RP2350-PICO2-HDR`.

## Initial system role

The hardware is intended to form one node of a multi-zone alarm system.

At this stage, no final pin assignment, electrical protection design, communication protocol decision, or firmware architecture decision has been made.
