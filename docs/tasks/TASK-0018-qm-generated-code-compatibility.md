# TASK-0018 - QM 5.3.0 generated-code compatibility check

## Status

Approved compatibility check / completed.

## Objective

Verify the user-installed QM 5.3.0 can generate C++ state-machine code compatible with the project's pinned QP/C++ v8.1.5 before implementing the planned graphical Blinky clone.

## Approval and scope

The user selected QM/QMC 5.3.0 for compatibility investigation and reported installing QM 5.3.0. Execution of the installed modeler in command-line generation mode and a project-local generation/compilation test are authorized. This does not authorize creating the final clone, changing the existing example/framework/BSP, installing additional tools, flashing, or committing.

## Planned changes and tests

- Record the installed tool version and available generation commands.
- Create an isolated minimal model and test sources under ignored `build/qm-compatibility/`.
- Generate code using the installed modeler; compile against the actual QP/C++ headers and configured host/Arm ports, without hand-editing generated code.
- Where practical, link/run a host smoke test against the existing real QP/QV host-test library.
- Record required compatibility settings, limitations, and remaining clone-design decisions.
- Register the task in `TASKS.md`.

## Completion criteria

- Demonstrate actual generation/compilation, not just a minimum-version claim.
- Keep QP/C++ v8.1.5 pinned and the existing application untouched.
- Distinguish the GUI modeler's command-line generation from standalone QMC availability.
- Record test coverage and untested behavior honestly.

## Initial findings

`/Applications/qm.app/Contents/MacOS/qm -h` reports `qm 5.3.0` and supports `-c` command-line code generation. The standalone `/Applications/qmc.app` was not found; the installed GUI bundle contains only `qm` in its MacOS directory. No additional installation is required to attempt generation with `qm -c`.

## Results

Confirmed the user-installed QM executable reports 5.3.0 and generates code from a project-local model using `qm -c`. No standalone QMC installation was needed.

Created an isolated probe model under ignored `build/qm-compatibility/` with two QActive/QHsm state machines:

- A hierarchical LED-like machine with off/on entry actions, a one-shot time event, and a payload-bearing configuration event handled in a parent state.
- A Console-like machine with a periodic poll event and an internal work/service event.

The initial handwritten probe XML needed missing transition glyphs added before QM accepted it. After correcting the model, QM generated one C++ file with zero warnings/errors. The generated C++ was not hand-edited. SVG export for both diagrams also succeeded.

Validation performed:

1. Compiled the generated C++ and a mock-I/O driver as C++17 using AppleClang and the existing real QP core/QV host library at v8.1.5, with AddressSanitizer/UndefinedBehaviorSanitizer enabled.
2. Set `QP_API_VERSION=815` explicitly for generated code and the driver: no older-API compatibility setting was needed for this probe.
3. Ran the host executable successfully. It verified both active objects, parent-state payload handling, initial/off/on transitions, one-shot timing without restarting the current phase, periodic polling, and internal work-event dispatch. The executable reported QP/C++ 8.1.5.
4. Compiled the same unchanged generated C++ to an Arm object using the actual Pico 2 W CMake/Ninja compiler flags, Cortex-M33 port, project QP configuration and QP/C++ v8.1.5 headers. The result is an ARM EABI5 object with debug information.
5. Exported `SM_of_BlinkProbe.svg` and `SM_of_ConsoleProbe.svg` using `qm -d '*.svg'` from the isolated directory.

Commands used (from the project root, except diagram export):

```bash
/Applications/qm.app/Contents/MacOS/qm build/qm-compatibility/probe.qm -c
build/qm-compatibility/host-probe
```

The host compile/link used `-std=c++17 -DQP_API_VERSION=815 -fsanitize=address,undefined`, project host/framework include paths, and `build/host/libqpcpp_host.a`. The Arm compile reused the Ninja command for the existing application, substituting only the generated source, output object/dependency locations, probe include directory and strict API-version definition.

Conclusion: **QM 5.3.0 QActive/QHsm code generation is compatible with the tested QP/C++ v8.1.5 features and the project's Arm compilation environment.** This is not a complete clone implementation, Arm firmware link/flash, hardware test, exhaustive generator certification, or QMActive/QMsm-strategy test. Full duty-cycle behavior, USB coalescing/session integration and source-regeneration reproducibility must still be validated when the clone is implemented.

All probe/model/generated/test/diagram artifacts remain under ignored `build/qm-compatibility/`. The existing Blinky, BSP, framework sources and dependency pin are unchanged. No installation, clone implementation, flashing, or commit occurred.

Recommended next design choices: retain the existing QActive/QHsm execution strategy for the modeled clone; use the tested `qm -c` command as the available generator unless standalone QMC is separately selected/installed. Obtain individual approvals for generated-source layout/workflow, build selection and clone implementation.
