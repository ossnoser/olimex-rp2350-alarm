# Hardware documentation

This directory contains hardware documentation for the embedded alarm node firmware project.

## Documents

- [Hardware baseline](hardware-baseline.md): active base hardware for the project.
- [Open points](open-points.md): hardware information still to be collected or clarified.
- [Target board decision](../decisions/DEC-0001-rp2350-pico2-hdr-target.md): formal decision for the firmware target board.

## Current project target

The project targets `RP2350-PICO2-HDR`.

The `-HDR` suffix identifies the version with soldered pin headers, suitable for insertion into the Olimex `PICO-EVB` socket.

In this project, `RP2350-PICO2-HDR` is always used mounted on the Olimex `PICO-EVB` carrier board.

Each node includes an Olimex `MOD-RS485` module and communicates with other identical nodes through RS485.
