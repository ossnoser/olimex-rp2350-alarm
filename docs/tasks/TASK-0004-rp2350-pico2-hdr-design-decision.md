# TASK-0004 - RP2350-PICO2-HDR target board design decision

## Status

Approved / completed.

## Objective

Clarify and document the project target hardware and create a formal design decision for the firmware target board.

## Confirmed clarifications

- The firmware target board is `RP2350-PICO2-HDR`.
- `RP2040-PICO` is the previous-generation model and is not part of the active hardware baseline.
- The `-HDR` suffix identifies the version with soldered pin headers, suitable for insertion into the `PICO-EVB` socket.
- The `RP2350-PICO2-HDR` board is always mounted on the `PICO-EVB` carrier board in this project.
- Each alarm node is composed of:
  - `RP2350-PICO2-HDR`;
  - `PICO-EVB`;
  - `MOD-RS485`.
- Multiple identical nodes are connected through RS485.
- The firmware project considers the node as powered from `5 V`.
- `PICO-EVB` inputs and outputs are generic firmware resources:
  - `input1` ... `input4`;
  - `output1` ... `output4`.

## Planned changes

Create:

- `docs/decisions/DEC-0001-rp2350-pico2-hdr-target.md`
- `docs/tasks/TASK-0004-rp2350-pico2-hdr-design-decision.md`

Update:

- `TASKS.md`
- `docs/hardware/README.md`
- `docs/hardware/hardware-baseline.md`
- `docs/hardware/open-points.md`

## Completion criteria

The task is complete when:

- `TASKS.md` lists `TASK-0004`;
- a formal design decision confirms the `RP2350-PICO2-HDR` target;
- the hardware baseline no longer treats `RP2040-PICO-HDR` as candidate baseline hardware;
- the baseline documents `RP2350-PICO2-HDR`, `PICO-EVB`, and `MOD-RS485` as mandatory node components;
- the baseline documents firmware-visible power as `5 V`;
- the baseline documents generic I/O resources `input1` ... `input4` and `output1` ... `output4`;
- no files outside the project directory have been modified.

## Result

Completed.
