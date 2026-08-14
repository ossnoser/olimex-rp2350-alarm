# TASK-0005 - Development toolchain and framework

## Status

Approved / completed.

## Objective

Document the intended software development environment, language preference, required tools, and application framework for the `RP2350-PICO2-HDR` firmware project.

## Confirmed inputs

- Preferred programming language: `C++`, if available and practical for the selected microcontroller and SDK.
- Acceptable fallback language: `C`, if C++ is not practical.
- Development host: macOS on Intel processor.
- Development editor/IDE: Visual Studio Code.
- Target application framework to integrate: Quantum Leaps `qpcpp`.
- Framework repository: <https://github.com/QuantumLeaps/qpcpp>.

## Planned changes

Create:

- `docs/development/README.md`
- `docs/development/toolchain.md`
- `docs/development/framework.md`
- `docs/decisions/DEC-0002-language-toolchain-framework.md`
- `docs/tasks/TASK-0005-development-toolchain-and-framework.md`

Update:

- `TASKS.md`
- `docs/README.md`

## Tool categories to document

- Host operating system requirements for macOS Intel.
- Visual Studio Code and recommended extensions.
- Build tools.
- ARM embedded C/C++ toolchain.
- Raspberry Pi Pico SDK with RP2350/Pico2 support.
- Flashing tools.
- Optional SWD debug tools.
- Quantum Leaps `qpcpp` integration considerations.

## Completion criteria

The task is complete when:

- `TASKS.md` lists `TASK-0005`;
- development documentation exists under `docs/development/`;
- the toolchain document lists the tools needed for macOS Intel development;
- the framework document records the intent to integrate `qpcpp`;
- a formal design decision records the language, toolchain, and framework direction;
- no files outside the project directory have been modified.

## Result

Completed.
