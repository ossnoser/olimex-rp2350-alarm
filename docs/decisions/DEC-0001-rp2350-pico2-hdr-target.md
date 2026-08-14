# DEC-0001 - RP2350-PICO2-HDR target board

## Status

Accepted.

## Context

The project implements embedded firmware for a multi-zone alarm node based on Olimex/Raspberry Pi Pico-compatible hardware.

The hardware discussion included previous-generation RP2040-based Pico boards and the newer RP2350-based Pico2 board family.

## Decision

The firmware target board for this project is `RP2350-PICO2-HDR`.

The board is always used mounted on the Olimex `PICO-EVB` carrier board.

Each alarm node is composed of the following mandatory hardware components:

- `RP2350-PICO2-HDR` target board;
- Olimex `PICO-EVB` carrier board;
- Olimex `MOD-RS485` communication module.

Multiple identical nodes are connected through an RS485 bus.

The firmware shall consider the node as powered from `5 V`. Any upstream system-level power conversion is outside the firmware project scope unless explicitly documented by a future task.

The `PICO-EVB` board resources are exposed to the firmware as generic resources:

- `input1`;
- `input2`;
- `input3`;
- `input4`;
- `output1`;
- `output2`;
- `output3`;
- `output4`.

No functional alarm-zone meaning is assigned to these resources at this stage.

## Clarifications

- `RP2040-PICO` is the previous-generation board family and is not part of the active hardware baseline.
- The `-HDR` suffix identifies the variant with soldered pin headers.
- The soldered pin headers allow insertion into the `PICO-EVB` socket.

## Consequences

- Firmware, pin mapping, build configuration, and board support decisions must target `RP2350-PICO2-HDR`.
- Documentation must not use RP2040-based boards as active baseline hardware.
- The `PICO-EVB` must be treated as part of the standard node hardware, not as an optional development board.
- `MOD-RS485` must be treated as part of the standard node hardware.
- Input and output naming in firmware documentation should use the generic resource names unless a future approved decision assigns application-specific roles.
