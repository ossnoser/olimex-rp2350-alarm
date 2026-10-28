# TASK-0012 - Pico 2 W QP/C++ Blinky integration plan

## Status

Approved planning documentation / completed.

The user approved documenting the decisions below. Firmware implementation, build files, dependency acquisition, and commits are not authorized by this task.

## Objective

Document the approved foundation for integrating QP/C++ with a standalone Raspberry Pi Pico 2 W using a minimal Blinky example, keeping BSP and application files separate. The later alarm application is outside this first integration step.

## Authorized changes

Initially approved: create this task document and register it in `TASKS.md` only.

Subsequent documentation-only approval (decision 18): update this plan with decisions 15-17, create `docs/tasks/TASK-0013-pico2-w-qpcpp-blinky-implementation.md`, and register TASK-0013 in `TASKS.md` as pending implementation approval. No source, build, dependency, or outside-project changes are authorized.

## Individually approved decisions

1. **Initial target:** Raspberry Pi Pico 2 W only. Non-wireless Pico 2 support is deferred.
2. **Hardware setup:** standalone board powered and flashed over USB, using its onboard LED. Olimex PICO-EVB and MOD-RS485 are excluded from this step.
3. **Separation:** `bsp/` contains hardware initialization, LED support, framework tick, and board-specific error handling; `examples/blinky/` contains the active object and state machine; `third_party/qpcpp/` contains the framework dependency. Application code uses a small BSP interface and contains no Pico SDK headers or hardware-specific implementation.
4. **Execution model:** cooperative QV kernel on core 0 only. Core 1 remains unused. Event handlers must be short and nonblocking.
5. **Dependency management:** Git submodule at `third_party/qpcpp/`, pinned to an exact stable release commit. The exact version, commit, and licensing require separate approval before acquisition.
6. **Build configuration:** C++17, Raspberry Pi Pico SDK, CMake, and Ninja; `PICO_BOARD=pico2_w`; `PICO_PLATFORM=rp2350-arm-s`; separate build directory `build/pico2_w/`; `.elf` and `.uf2` artifacts. Check the existing SDK installation against the selected framework release.
7. **Application behavior:** one Blinky active object with explicit LED-on and LED-off states, changing state every 500 ms through a QP time event. One complete cycle per second, with no blocking delays or networking.
8. **Framework tick:** 100 Hz (10 ms) from a Pico SDK hardware timer interrupt. The interrupt advances QP time events; timeout dispatch and LED control occur in normal application context. Verify the exact timer integration against the selected port.
9. **Wireless-chip LED support:** use `pico_cyw43_arch_none` for CYW43 initialization and LED control, entirely within the BSP. No Wi-Fi connection or networking stack.
10. **Errors:** fail-stop on QP assertions or unrecoverable initialization failures; retain diagnostic information for debugger inspection; no automatic reset. Watchdog recovery is deferred.
11. **Observability:** no USB/UART logging and no QP QS tracing initially. LED behavior indicates normal operation; debugger inspection supports fault diagnosis.
12. **Release-selection process:** review available stable QP/C++ releases for Cortex-M33/QV port and API compatibility. Present exact version, pinned commit, and licensing requirements for separate approval before adding the dependency. Review alone does not authorize acquisition.
13. **Validation:** require a clean CMake/Ninja build producing ELF and UF2, then USB flashing and continuous physical blinking with state changes every 500 ms. If hardware testing is unavailable, report build-verified only, not fully validated.
14. **Documentation gate:** create this task document and update the task index only. No firmware implementation, build files, or dependency changes yet.
15. **Exact framework version:** QP/C++ v8.1.5, pinned to commit `198b47e7169e27f41d761868fbe83e08d624cd5d`. Upstream release: <https://github.com/QuantumLeaps/qpcpp/releases/tag/v8.1.5>. Its C++17 requirement matches the approved language standard; `ports/arm-cm/qv/gnu/` is the candidate GNU Arm QV port. Pico SDK/RP2350 compatibility remains to be validated.
16. **Framework licensing:** use QP/C++ under GPL-3.0-or-later, preserving license and copyright notices. No commercial license selected. Distribution must comply with the applicable GPL obligations, including corresponding source requirements.
17. **New project code licensing:** new Blinky application, BSP, and integration code will use GPL-3.0-or-later. This does not relicense existing repository content or third-party dependencies.
18. **Implementation-task documentation:** record decisions 15-17 here and prepare TASK-0013 as pending implementation approval. This approval permits documentation changes only.

## Intended implementation sequence (not yet authorized)

1. Review upstream QP/C++ release, port, licensing, and SDK compatibility without adding the dependency.
2. Obtain approval of the exact release, commit, and licensing basis.
3. Document the implementation task and obtain explicit implementation approval.
4. Establish build integration, the separated BSP, and minimal Blinky application.
5. Build and validate on the standalone Pico 2 W.
6. Document reproducible setup, build, flashing, and actual validation results.

## Pending decisions and checks

- Exact release, commit, and licensing basis have been approved (decisions 15-17); actual dependency acquisition is still gated by implementation approval.
- Exact upstream Cortex-M33/QV port and compatibility with Pico SDK startup and interrupt handling.
- Exact timer implementation, framework configuration, startup ordering, and idle behavior.
- Exact BSP API and source-file layout within the approved directory boundaries.
- Explicit approval to begin implementation.

These are pending details, not assertions that compatibility has already been verified.

## Relationship to existing documentation

The earlier hardware baseline and integration guide describe an Olimex RP2350-PICO2-HDR on PICO-EVB. This approved first integration instead targets a standalone Raspberry Pi Pico 2 W. Those existing documents are not modified by this documentation-only task. Broader hardware-baseline changes remain outside this approval.

## Completion criteria

- This task records all individually approved choices and remaining approval gates.
- `TASKS.md` lists TASK-0012 and links to this document.
- No firmware sources, build configuration, dependencies, or unrelated files are changed.

## Result

Planning documentation completed, including the separately approved release and licensing decisions. TASK-0013 defines the proposed implementation scope. Implementation has not started and remains subject to approval.
