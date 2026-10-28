# TASK-0011 - QP/C++ integration guide planning

## Status

Approved / completed.

## Objective

Create a planning guide for the future integration of Quantum Leaps QP/C++ (`qpcpp`) into the Pico SDK firmware project.

The guide will be used later as the implementation reference for creating an initial working LED blinking firmware example based on `qpcpp`.

## Scope

This task does not integrate `qpcpp` and does not create firmware source code.

This task creates the documentation baseline that will guide a future implementation task.

## Planned changes

Create:

- `docs/development/qpcpp-integration-guide.md`
- `docs/tasks/TASK-0011-qpcpp-integration-guide-planning.md`

Update:

- `TASKS.md`
- `docs/development/README.md`

## Guide purpose

The guide must describe how to obtain a minimal `qpcpp`-based LED blinking firmware example.

The example will demonstrate that:

- Pico SDK builds correctly;
- C++ builds correctly;
- `qpcpp` is integrated correctly;
- a minimal event-driven active-object application can run on `RP2350-PICO2-HDR`;
- future firmware development can be based on `qpcpp`.

## Required guide content

The guide must include:

- goal and acceptance criteria;
- target hardware assumptions;
- development environment assumptions;
- `qpcpp` acquisition options;
- `qpcpp` version selection points;
- proposed repository layout;
- minimal firmware architecture;
- Pico SDK integration points;
- `qpcpp` integration points;
- manual user steps, if required;
- build instructions;
- flash/run instructions;
- open points before implementation.

## Completion criteria

The task is complete when:

- `TASKS.md` lists `TASK-0011`;
- `docs/development/qpcpp-integration-guide.md` exists and is written in English;
- `docs/development/README.md` links to the guide;
- the guide can be used as the basis for a future implementation task;
- no files outside the project directory have been modified.

## Result

Completed.
