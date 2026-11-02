# Hardware documentation

This directory contains hardware documentation for the embedded alarm node firmware project.

## Documents

- [Hardware baseline](hardware-baseline.md): active base hardware for the project.
- [PICO-EVB Rev. A integration investigation](pico-evb-rev-a-integration.md): verified-source GPIO/power routing, startup implications, UEXT conflicts and gated first-test plan; limited observed results are recorded in TASK-0022.
- [Diagnostic and individual relay commands](../../examples/pico-evb-diagnostic/README.md): current TASK-0028 uses `on N`, `off N` and global `off`, at most one latched ON, no timeout or lockout; detected session loss/suspend forces all OFF. Automated-tested and approved flash/startup verified; one approved REL1 ON/selected-OFF trace passed, physical REL1 latched ON/selected-OFF and broader all-four control/busy rejection/global OFF were user-reported successful. Unpowered/unwired cleanup user-confirmed; wrong-channel selected-OFF/loss/fault/current/thermal/contact qualification is not implied. Prior TASK-0027's one-shot R1..R4 sequence passed user-observed order/pattern and power-off cleanup, not continuous coil/contact/current/thermal qualification.
- [Relay power and single-test planning](relay-power-and-single-test-plan.md): LEG-5F coil/driver research, actual 250 mA USB declaration and a candidate single-coil policy; no actuation authorized.
- [Open points](open-points.md): hardware information still to be collected or clarified.
- [Target board decision](../decisions/DEC-0001-rp2350-pico2-hdr-target.md): formal decision for the firmware target board.

## Current project target

The project targets `RP2350-PICO2-HDR`.

The `-HDR` suffix identifies the version with soldered pin headers, suitable for insertion into the Olimex `PICO-EVB` socket.

In this project, `RP2350-PICO2-HDR` is always used mounted on the Olimex `PICO-EVB` carrier board.

Each node includes an Olimex `MOD-RS485` module and communicates with other identical nodes through RS485.
