# TASK-0003 - Hardware baseline documentation

## Status

Approved / completed.

## Objective

Add the initial hardware baseline documentation for the embedded alarm system project.

The documentation is based on the external source note provided by the user:

```text
/Users/ossnoser/Library/CloudStorage/OneDrive-Personal/Knowledge-base/10_Projects/Italy Home Alarm Renewal - Via Udine 55/Notes/Candidate Replacement Hardware.md
```

The source file is read-only for this project task and must not be modified.

## Planned changes

Create or update the following project files:

- `TASKS.md`
- `docs/hardware/README.md`
- `docs/hardware/hardware-baseline.md`
- `docs/hardware/open-points.md`
- `docs/tasks/TASK-0003-hardware-baseline-documentation.md`

## Content to document

The hardware baseline must include:

- Olimex `PICO-EVB` evaluation board;
- compatibility with Raspberry Pi `PICO2` / RP2350-based module;
- four optoisolated inputs on `PICO-EVB`;
- four relay outputs on `PICO-EVB`;
- UEXT connector availability;
- Olimex `MOD-RS485` module;
- Amazon item / ASIN `B0BN1N88F6` selected for `12 V` to `5 V` conversion;
- open points and information still to be collected;
- clarification note about the project target `RP2350-PICO2` versus the source note also mentioning `RP2040-PICO-HDR`.

## Completion criteria

The task is complete when:

- `TASKS.md` lists `TASK-0003`;
- the hardware documentation directory contains an index README;
- the hardware baseline is documented in English;
- open points are documented in English;
- no files outside the project directory have been modified.

## Result

Completed.
