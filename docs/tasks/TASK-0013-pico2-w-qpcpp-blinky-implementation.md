# TASK-0013 - Pico 2 W QP/C++ Blinky implementation

## Status

Approved / completed.

Preparation of this document was approved in decision 18 of TASK-0012. In decision 19, the user approved beginning TASK-0013, including implementation and acquisition of the already approved dependency. Remaining design details must still be presented individually for approval before implementation. Hardware flashing was subsequently explicitly authorized by the user for the guided USB BOOTSEL test. After hardware validation, the user explicitly requested committing this work.

## Objective

Implement a minimal QP/C++ Blinky example on a standalone Raspberry Pi Pico 2 W, establishing reusable build and board support while keeping application logic independent of the Pico SDK.

## Approved design baseline

Follow the individually approved decisions in [TASK-0012](TASK-0012-pico2-w-qpcpp-blinky-plan.md):

- Standalone Pico 2 W over USB; onboard CYW43-controlled LED.
- C++17, Pico SDK, CMake/Ninja, `PICO_BOARD=pico2_w`, `PICO_PLATFORM=rp2350-arm-s`.
- QP/C++ v8.1.5 at `198b47e7169e27f41d761868fbe83e08d624cd5d`, as a Git submodule at `third_party/qpcpp/`.
- QV cooperative scheduling on core 0; core 1 unused.
- One Blinky active object with LED-on/off states, changing state every 500 ms through a QP time event.
- 100 Hz tick driven by a hardware timer interrupt; dispatch and LED operations outside interrupts.
- BSP-owned `pico_cyw43_arch_none` initialization and LED control; no networking.
- Fail-stop error handling retaining debugger-visible diagnostics; no automatic reset.
- No USB/UART logging or QS tracing.
- GPL-3.0-or-later for the framework and new application, BSP, and integration code; preserve upstream notices and existing third-party licensing.

## Planned changes

Subject to explicit implementation approval:

1. Inspect the existing SDK/toolchain and selected upstream port; verify Cortex-M33, startup, interrupt, and C++ compatibility.
2. Add the approved QP/C++ submodule and immutable pin. Preserve upstream source and notices; avoid unnecessary nested dependency downloads.
3. Establish top-level CMake setup and framework build integration, separating framework/port configuration from application behavior.
4. Implement the BSP interface and Pico 2 W implementation under `bsp/`, including LED initialization/control, framework tick, idle integration, and error handling.
5. Implement the shared-style application under `examples/blinky/`, with no Pico SDK includes, pin numbers, or CYW43 APIs in application logic.
6. Disable stdio and tracing, generate ELF/UF2 artifacts under `build/pico2_w/`, and ignore generated outputs.
7. Add appropriate GPL notices and the applicable license text for new code without relicensing unrelated existing files.
8. Document dependency initialization, build, flashing, architecture boundaries, and actual validation results.
9. Update the task index/status and integration guide to distinguish this standalone proof of concept from the existing Olimex hardware baseline.

## Expected file areas

- `CMakeLists.txt` and, if needed, `cmake/` for build integration.
- `.gitmodules` and `third_party/qpcpp/` for the pinned dependency.
- `bsp/` for board support and framework hardware hooks.
- `examples/blinky/` for application behavior and composition/startup.
- A dedicated framework configuration location outside application behavior code.
- `.gitignore` for generated files.
- License text/notices scoped to newly written code.
- `docs/development/qpcpp-integration-guide.md` and relevant development documentation links/instructions.
- This task document and `TASKS.md` for status and results.

Exact file names, BSP API, configuration placement, timer mechanism, initialization ordering, and idle/interrupt policy remain design details to present individually for approval before implementing those choices. If the selected port conflicts with the SDK, stop and request approval of any design change rather than silently changing the baseline.

## Additional individually approved design details

20. **Framework configuration location:** `framework/qpcpp/CMakeLists.txt` and `framework/qpcpp/qp_config.hpp` hold framework build integration and configuration, separate from unchanged upstream sources, BSP, and application behavior.

21. **Application-facing BSP interface:** `bsp/include/bsp.hpp` exposes `BSP::init()` and `BSP::setLed(bool on)` without Pico SDK or QP types. Startup calls initialization; Blinky state-entry actions request LED changes. Timer and framework callbacks remain internal to the BSP.

22. **BSP file layout:** `bsp/CMakeLists.txt`, `bsp/include/bsp.hpp`, `bsp/pico2_w/bsp.cpp` for CYW43 initialization/LED control, and `bsp/pico2_w/qpcpp_hooks.cpp` for timer tick, framework callbacks, idle, and errors. Keep implementation board-specific initially; extract common code when another board is introduced.
23. **Startup sequence:** call `QP::QF::init()`, then `BSP::init()`, start Blinky, and enter `QP::QF::run()`. Start the 100 Hz timer from the QP startup callback after the active object is ready.

24. **Timer mechanism:** Pico SDK repeating timer with a negative 10,000 microsecond interval for start-to-start scheduling. Its short interrupt callback advances QP time events only; no LED operations. Fail-stop if timer creation fails. Check compatibility with the QP interrupt policy before implementation.

25. **Interrupt protection:** use the upstream QV port's PRIMASK-based critical sections with `QF_USE_BASEPRI` undefined. Verify SDK timer callback compatibility with the port's non-nesting checks; request approval before changing this policy if incompatibilities arise.

26. **Idle behavior:** call the upstream `QV_CPU_SLEEP()` macro from QV's idle callback, using its interrupt-enable/sleep sequencing. No CYW43 polling or LED operations in the idle callback.

27. **Event storage:** statically allocate one Blinky active object, one QP time-event object, and an eight-entry event queue. No dynamic event pools or application heap allocation. Use guaranteed event posting so queue overflow invokes fail-stop handling rather than silently discarding timeouts. SDK-internal allocation is not covered by the application allocation constraint.

28. **Initial LED state:** BSP initialization sets the LED off. Blinky starts in its LED-off state, transitions on after 500 ms, off after another 500 ms, and repeats.

29. **Application file layout:** `examples/blinky/CMakeLists.txt`, `main.cpp` for composition/startup, `blinky.hpp` for the application interface, and `blinky.cpp` for the active object and state machine.

30. **Shared timing constant:** expose `inline constexpr unsigned TICKS_PER_SECOND = 100U;` in namespace `BSP` in `bsp.hpp`. Derive the timer interval from this constant and the 500 ms Blinky interval as `BSP::TICKS_PER_SECOND / 2U`.

31. **Minimal QP configuration:** one supported active object, Blinky at priority 1; one time-event tick rate; assertions enabled in all configurations, including release builds. Expand later when the application requires additional resources.

32. **Fault diagnostics:** record fault source and error identifier in debugger-visible static variables, disable interrupts, and loop indefinitely. Do not call LED/logging APIs, reset automatically, or execute an unconditional breakpoint in the fault handler.

33. **Initial build type:** Debug, retaining debug symbols and assertions with optimization minimized. Validate release builds later, after initial Debug validation.

34. **License text location:** store the complete GPL license text at `LICENSES/GPL-3.0-or-later.txt` and identify new code with SPDX license identifiers. This does not relicense unrelated repository content.

35. **Framework build target:** build QP/C++ as a dedicated static library using an explicit list of required framework, QV kernel, and GNU Arm port sources. Exclude unused kernels and tracing components; do not modify upstream files.

36. **Blinky startup interface:** `blinky.hpp` exposes only `Blinky::start()`. Keep the active-object class, eight-entry queue, and time event private in `blinky.cpp`; `main.cpp` calls the startup function without managing QP storage.

37. **Board-selection guard:** CMake rejects any target other than `PICO_BOARD=pico2_w` with `PICO_PLATFORM=rp2350-arm-s` until another BSP is explicitly supported.

## Constraints

- Modify files only inside this project. Do not change the external SDK or toolchain installation.
- Preserve pre-existing unrelated work, including uncommitted documentation.
- Do not launch core 1 or introduce an RTOS, networking, alarm logic, RS485, persistence, watchdog recovery, or carrier-board integration.
- Application code must depend on a small BSP interface, not Pico SDK hardware APIs.
- Do not automatically relicense existing repository content or dependencies.
- Git commit approval was obtained separately after hardware validation. Preserve unrelated pre-existing changes when committing.
- Coordinate actual USB flashing with the user; do not treat build success as hardware validation.

## Validation plan

1. Configure from a clean project-local build directory using the documented SDK environment, board, platform, and Ninja generator.
2. Compile and link the framework, GNU Arm QV port, BSP, and Blinky with C++17; produce nonempty `.elf` and `.uf2` artifacts.
3. Check the dependency commit, source boundaries, tick rate, timeout interval, and absence of blocking application delays or enabled stdio/QS.
4. Provide USB BOOTSEL flashing instructions and coordinate hardware testing with the user.
5. Confirm repeated LED-on/off transitions every 500 ms on the standalone Pico 2 W and record the observation and test limitations.

## Completion criteria

- Reproducible clean CMake/Ninja build for `pico2_w` on `rp2350-arm-s`.
- QP/C++ is pinned to the approved commit and runs through the chosen QV port.
- ELF and UF2 artifacts exist in `build/pico2_w/` (exact subpaths documented).
- Blinky behavior is controlled by QP state transitions and time events.
- BSP and application files remain separate; application sources contain no Pico SDK hardware dependencies.
- Physical board blinking is verified, or status is explicitly recorded as build-verified / hardware validation pending. Full validation requires the hardware test.
- GPL license text and notices for new code and upstream notices are preserved appropriately.
- Setup, build, flashing, and actual test results are documented.
- No outside-project or unrelated changes are made.

## Result

Implementation approved in decision 19. Additional approved design details are recorded above. QP/C++ has been added as a Git submodule at the approved commit; nested submodules have not been initialized. The external Pico SDK reports version 2.3.0. Implemented the approved BSP, framework configuration/static-library target, application state machine, startup, build guards, and GPL notices/text. The integration guide now documents the standalone target and reproducible build/flash workflow.

The project configured from a new `build/pico2_w/` directory and built successfully with Pico SDK 2.3.0 and GNU Arm 14.2.1. Reconfigured and rebuilt with `PICO_DEOPTIMIZED_DEBUG=1` to honor the approved minimal-optimization Debug policy. Generated `build/pico2_w/examples/blinky/qpcpp-blinky.elf` and `.uf2`. Symbol inspection confirmed the QP startup/idle callbacks, fail-stop handler, and fault diagnostic variables are linked. `git diff --check` passed.

Hardware validation completed: the user connected the standalone Pico 2 W in BOOTSEL mode and explicitly authorized checking the boot volume and copying the firmware. `/Volumes/RP2350/INFO_UF2.TXT` identified an RP2350 bootloader. The agent copied the generated UF2 to that volume, which disconnected afterward, consistent with automatic reboot. The user then confirmed continuous onboard LED blinking at the expected approximately 500 ms on / 500 ms off cadence.

Status: build-verified and hardware smoke-test validated on the standalone Pico 2 W. Cadence was visually confirmed by the user, not instrument-measured; exhaustive timing, stress, fault-injection, and release-build testing were not performed. The user subsequently authorized committing the completed integration and its planning/validation records.
