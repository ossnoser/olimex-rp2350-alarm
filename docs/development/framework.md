# Application framework

## Selected framework direction

The project intends to integrate Quantum Leaps `qpcpp` as the application framework.

Repository:

- <https://github.com/QuantumLeaps/qpcpp>

## Language preference

The preferred project language is `C++`, assuming it is practical with the selected RP2350/Pico SDK toolchain.

`C` is an acceptable fallback only if C++ support proves impractical or unsuitable for the target, build system, or framework integration.

## Expected role of qpcpp

`qpcpp` is expected to provide an event-driven active-object framework for the firmware application.

Potential uses in this project include:

- modeling each alarm node as an event-driven embedded application;
- separating input processing, output control, RS485 communication, and supervision logic;
- defining explicit state machines for alarm-node behavior;
- improving determinism and testability of application-level logic.

## Integration options to evaluate

A future implementation task must decide how `qpcpp` is integrated into the repository and build system.

Candidate approaches:

- Git submodule pointing to the upstream `qpcpp` repository.
- Vendored source snapshot under a project-controlled third-party directory.
- External dependency checked out by setup scripts or documented manual steps.

No approach has been selected yet.

## Build integration considerations

The future build system must define:

- how `qpcpp` sources are included in the CMake build;
- which `qpcpp` port, configuration, or kernel option is suitable for RP2350/Pico SDK;
- whether the project uses bare-metal cooperative scheduling, preemptive scheduling, or another supported execution model;
- how assertion, tracing, logging, and error handling are configured;
- how `qpcpp` interacts with Pico SDK startup, timers, interrupts, and multicore features, if any.

## Open points

- Confirm that `qpcpp` supports the selected RP2350/Pico SDK build model.
- Select the exact `qpcpp` version or commit.
- Select the integration method.
- Decide whether the project uses only one RP2350 core or both cores.
- Define the event model for:
  - `input1` ... `input4`;
  - `output1` ... `output4`;
  - RS485 receive/transmit handling;
  - node supervision and watchdog handling.
- Define coding standards and C++ language version.
