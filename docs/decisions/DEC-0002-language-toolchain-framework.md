# DEC-0002 - Language, toolchain, and application framework

## Status

Accepted.

## Context

The project implements firmware for `RP2350-PICO2-HDR` nodes mounted on `PICO-EVB` carrier boards and connected through `MOD-RS485` modules.

The development host is macOS on an Intel processor, and the selected development editor is Visual Studio Code.

The firmware should preferably be implemented in C++ if supported by the target microcontroller, SDK, and selected framework.

## Decision

The preferred implementation language is `C++`.

`C` is accepted only as a fallback if C++ is not practical for the selected RP2350/Pico SDK environment or framework integration.

The development environment is based on:

- macOS on Intel processor;
- Visual Studio Code;
- CMake;
- Ninja or Make, with Ninja preferred;
- ARM GNU embedded toolchain;
- Raspberry Pi Pico SDK with RP2350/Pico2 support.

The application framework to integrate is Quantum Leaps `qpcpp`:

- <https://github.com/QuantumLeaps/qpcpp>

## Required tool categories

The project requires tools for:

- source control;
- CMake-based embedded builds;
- ARM C/C++ compilation;
- Pico SDK integration;
- UF2 and/or `picotool` flashing;
- optional SWD debugging;
- VS Code editing, build, and debug integration.

## Consequences

- Future source code and build scripts should assume C++ as the preferred language.
- Toolchain setup documentation must target macOS Intel.
- Future build configuration must evaluate and integrate `qpcpp`.
- The final `qpcpp` integration method must be decided by a future approved task.
- The final Pico SDK version, board definition, and flashing/debug workflow must be decided by future approved tasks.
