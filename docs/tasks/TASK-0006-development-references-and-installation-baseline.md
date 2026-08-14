# TASK-0006 - Development references and installation baseline

## Status

Approved / completed.

## Objective

Collect official references for the selected hardware, development tools, SDK, debug/flash utilities, and application framework. Use these references to create a baseline list of software that should be installed on the macOS Intel development PC before firmware development starts.

## Confirmed scope

References must cover:

- `RP2350-PICO2-HDR` target board;
- Olimex `PICO-EVB` carrier board;
- Olimex `MOD-RS485` module;
- Raspberry Pi Pico SDK with RP2350/Pico2 support;
- Visual Studio Code workflow;
- CMake/Ninja/Python/Git;
- ARM GNU embedded toolchain;
- `picotool`;
- optional OpenOCD/SWD debugging;
- Quantum Leaps `qpcpp`.

## Planned changes

Create:

- `docs/development/references.md`
- `docs/development/installation-baseline.md`
- `docs/tasks/TASK-0006-development-references-and-installation-baseline.md`

Update:

- `TASKS.md`
- `docs/development/README.md`

## Completion criteria

The task is complete when:

- `TASKS.md` lists `TASK-0006`;
- official or primary references are documented;
- an installation baseline for macOS Intel is documented;
- required, recommended, optional, and open-point tools are separated;
- no files outside the project directory have been modified.

## Result

Completed.
