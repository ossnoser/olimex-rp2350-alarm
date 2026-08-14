# Development environment setup verification

This document records the verified development environment status for the macOS Intel development PC.

## Verification date

2026-08-14.

## Host environment

- Host OS family: macOS.
- Processor architecture: Intel.
- Project path: `/Users/ossnoser/Documents/prj/olimex-rp2350-alarm`.
- Shared Pico workspace path: `/Users/ossnoser/Documents/prj/pico`.

## Verified base tools

### Git

Status: OK.

Observed version:

```text
git version 2.50.1 (Apple Git-155)
```

### CMake

Status: OK.

Observed version:

```text
cmake version 4.4.2
```

### Ninja

Status: OK.

Observed version:

```text
1.13.2
```

### Python 3

Status: OK.

Observed version:

```text
Python 3.14.6
```

### Homebrew

Status: OK.

Observed version:

```text
Homebrew 6.0.6
```

## Visual Studio Code

Status: OK, user-confirmed.

The required/recommended VS Code extensions from the onboarding guide were reported as installed by the user.

## ARM GNU embedded toolchain

Status: OK.

The initial Homebrew `arm-none-eabi-gcc` package was incomplete for this project because it did not provide the required standard C library headers/libraries. It was replaced with a complete Arm GNU Toolchain installation.

Verified tools:

```text
arm-none-eabi-gcc
arm-none-eabi-g++
arm-none-eabi-gdb
arm-none-eabi-objcopy
arm-none-eabi-size
```

Observed versions:

```text
arm-none-eabi-gcc  Arm GNU Toolchain 14.2.Rel1
arm-none-eabi-g++  Arm GNU Toolchain 14.2.Rel1
arm-none-eabi-gdb  GNU gdb 15.2.90, Arm GNU Toolchain 14.2.Rel1
arm-none-eabi-objcopy  GNU objcopy 2.43.1, Arm GNU Toolchain 14.2.Rel1
arm-none-eabi-size  GNU size 2.43.1, Arm GNU Toolchain 14.2.Rel1
```

Toolchain location observed through GCC output:

```text
/Applications/ArmGNUToolchain/14.2.rel1/arm-none-eabi
```

Standard header checks:

```text
stdio.h: OK
assert.h: OK
```

## Pico SDK

Status: OK.

SDK path configured by the project environment script:

```text
PICO_SDK_PATH=/Users/ossnoser/Documents/prj/pico/pico-sdk
```

The SDK directory exists and is usable.

Observed SDK Git commit:

```text
98a542c1
```

## Project environment script

Status: OK.

The project root script:

```text
env.sh
```

correctly sets:

```bash
export PICO_SDK_PATH=/Users/ossnoser/Documents/prj/pico/pico-sdk
```

Usage:

```bash
source env.sh
```

## picotool

Status: OK.

CMake detected `picotool` at:

```text
/usr/local/bin/picotool
```

## pico-examples

Status: OK.

Examples path:

```text
/Users/ossnoser/Documents/prj/pico/pico-examples
```

Observed Git commit:

```text
c81c855
```

The examples were configured for:

```text
PICO_BOARD=pico2
PICO_PLATFORM=rp2350-arm-s
```

CMake configuration succeeded and used the board configuration:

```text
/Users/ossnoser/Documents/prj/pico/pico-sdk/src/boards/include/boards/pico2.h
```

The final build generated firmware artifacts successfully, including `.uf2` files.

Observed result:

```text
164 .uf2 files generated
```

Example generated firmware files:

```text
blink/blink.uf2
pio/hello_pio/hello_pio.uf2
sha/mbedtls_sha256/mbedtls_sha256.uf2
usb/host/host_cdc_msc_hid/host_cdc_msc_hid.uf2
```

## Conclusion

The development environment is operational for building RP2350/Pico2 firmware using the Raspberry Pi Pico SDK, CMake, Ninja, and the Arm GNU Toolchain.

The environment is ready for the next project phase: creating the first project-local CMake/Pico SDK firmware skeleton.

## Remaining optional items

The following items are not required for the first firmware build and remain optional or future work:

- SWD debug probe selection.
- OpenOCD RP2350 debug configuration.
- VS Code Cortex-Debug configuration.
- Final `qpcpp` integration method and version selection.
