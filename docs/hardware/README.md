# Hardware documentation

This directory contains hardware documentation for the embedded alarm node firmware project.

## Documents

- [Hardware baseline](hardware-baseline.md): active base hardware for the project.
- [PICO-EVB Rev. A integration investigation](pico-evb-rev-a-integration.md): verified-source GPIO/power routing, startup implications, UEXT conflicts and gated first-test plan; limited observed results are recorded in TASK-0022.
- [Diagnostic and four-relay one-shot example](../../examples/pico-evb-diagnostic/README.md): prior output-only/input checks and REL1-only pulse/repeat observations passed. Current `testall` sequence runs R1..R4, one second each with one-second all-OFF gaps, then locks OFF; automated-tested and approved flash/startup verified; one approved manual sequence trace matched R1..R4 and final LOCKED/all-OFF; the user confirmed the physical order and approximate one-second ON/OFF pattern. Power-off/unwired cleanup was user-confirmed; no load/contact/current/transient qualification is implied.
- [Relay power and single-test planning](relay-power-and-single-test-plan.md): LEG-5F coil/driver research, actual 250 mA USB declaration and a candidate single-coil policy; no actuation authorized.
- [Open points](open-points.md): hardware information still to be collected or clarified.
- [Target board decision](../decisions/DEC-0001-rp2350-pico2-hdr-target.md): formal decision for the firmware target board.

## Current project target

The project targets `RP2350-PICO2-HDR`.

The `-HDR` suffix identifies the version with soldered pin headers, suitable for insertion into the Olimex `PICO-EVB` socket.

In this project, `RP2350-PICO2-HDR` is always used mounted on the Olimex `PICO-EVB` carrier board.

Each node includes an Olimex `MOD-RS485` module and communicates with other identical nodes through RS485.
