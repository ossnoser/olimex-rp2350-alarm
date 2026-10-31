# TASK-0020 - Pico 2 and Pico 2 W support

## Status

Approved / completed; builds, host checks and all four hardware smoke combinations passed.

## Objective

Support standard non-wireless Pico 2 and Pico 2 W in both the handwritten and QM-modeled duty-cycle examples, with board-specific hardware kept in the BSP.

## Individually approved decisions

1. The user clarified that official Pico 2 and Olimex RP2350-PICO2-HDR use the same `pico2` software target; the HDR variant adds pre-mounted headers for the PICO-EVB.
2. Retain both `pico2` and `pico2_w`, selected at build time, for both examples.
3. Separate `bsp/pico2/` GPIO LED support and `bsp/pico2_w/` CYW43 LED support; extract shared USB CDC, descriptors, tick and QP hooks to `bsp/common/`. Keep the application interface unchanged.
4. Default new configurations to `pico2`; use separate `build/pico2/` and `build/pico2_w/` directories, both `rp2350-arm-s`. Each produces separate original/QM images.
5. Validate standalone over USB; carrier-board inputs/relays and RS485 remain outside this change.
6. USB product names: `Pico 2 Blinky` and `Pico 2 W Blinky`. Retain CDC protocol, development VID/PID and board-unique serial number.
7. Both boards are available for eventual physical validation; build all four firmware combinations and distinguish host/build checks from board observations.
8. Document and implement this scope, update build/terminal instructions, and preserve application/QM behavior. Each board flashing and any commit require separate approval.

## Planned changes

- Refactor the BSP into common USB/framework support plus one selected LED implementation. Pico 2 uses SDK `PICO_DEFAULT_LED_PIN`; Pico 2 W retains CYW43 with no networking.
- Select only the required board-specific sources/libraries. Non-wireless builds must not compile/link CYW43 support.
- Broaden board guard to the two approved boards, retain the Arm platform guard, and default only new build configurations to Pico 2.
- Build both images in each separate board directory; check actual source/link graphs and product-name strings.
- Rerun original/clone host tests and QM freshness checks without changing application sources or the model.
- Add targeted LED adapter tests with SDK stubs where useful; do not confuse those with physical GPIO/USB tests.
- Update relevant documentation and record actual results while preserving existing uncommitted QM-clone work.

## Completion criteria

- Both examples build for each board, generating four separate ELF/UF2 pairs.
- New default and explicit board selections are correct; unsupported boards/platforms fail configuration.
- Standard Pico 2 initializes/drives only its direct LED GPIO and has no CYW43 dependency. Pico 2 W preserves its LED behavior.
- USB identity matches the selected board; common USB/event/tick/error behavior remains unchanged.
- Existing host/freshness checks and targeted new tests pass; application sources and QM model/generated files remain unchanged.
- Hardware validation is recorded separately for each example/board after explicit flashing approval. No unapproved commit or outside-project source modification.

## Hardware validation matrix

| Board | Example | Post-refactor status |
|---|---|---|
| Pico 2 | Handwritten Blinky | Passed: USB identity/enumeration, duty-cycle console/LED, invalid input, reconnect and reboot defaults |
| Pico 2 | QM-modeled Blinky | Passed: USB identity/enumeration, duty-cycle console/LED, invalid input, reconnect and reboot defaults |
| Pico 2 W | Handwritten Blinky | Passed: USB identity/enumeration, duty-cycle console/LED, invalid input, reconnect and reboot defaults |
| Pico 2 W | QM-modeled Blinky | Passed: USB identity/enumeration, duty-cycle console/LED, invalid input, reconnect and reboot defaults |

Observations are user-confirmed smoke checks, not instrumented timing measurements.

## Result

Implemented the shared BSP plus selected LED adapter. `bsp/common/` now contains initialization/USB transport, descriptors, TinyUSB configuration and QP/tick/error hooks. `bsp/pico2/led.cpp` uses the SDK LED pin macro, sets the off level before enabling output, and supplies `Pico 2 Blinky`; `bsp/pico2_w/led.cpp` retains CYW43 initialization/control and supplies `Pico 2 W Blinky`. The application-facing BSP interface is unchanged.

Top-level CMake defaults new configurations to `pico2`, allows the two approved board names on `rp2350-arm-s`, and selects only the appropriate sources/libraries. Existing wireless caches retain their board. Added the board-support guide and updated integration/clone instructions for the four-image matrix and board-specific USB identities.

Validation performed:

- Configured `build/pico2/` without a board override, confirming the new non-wireless default, and built both examples. Explicitly configured `build/pico2_w/` and built both examples there.
- `tests/bsp/verify_builds.py` passed all four board/example combinations: correct board/platform caches and LED sources, common USB descriptors, no standard-board CYW43 source or linked initialization symbol, required wireless dependencies present, USB/UART stdio absent, correct product literals and nonempty ELF/UF2 files.
- Added three sanitizer-enabled native LED-adapter tests: Pico 2 GP25, a pin-macro override proving no hardcoded pin, and Pico 2 W control/identity plus CYW43 failure handling. All passed.
- Reran all four original host tests and all six clone behavior/generator tests; all passed. This is 13 CTest cases total (11 sanitizer-enabled C++ cases and two Python/QM checks), in addition to build-graph/guard checks.
- Configuration correctly rejected an unsupported board and the RISC-V platform.
- Compared moved QP hooks/TinyUSB configuration byte-for-byte against the prior files: unchanged. Compared the shared USB transport against the original with only the explicit LED initialization/extraction substitutions: unchanged otherwise.
- Verified original application sources, public BSP header, framework and QP dependency are unchanged. The QM model/generated application code were not edited; generation freshness checks passed.

The automated results above do not themselves establish physical pin/USB/timing behavior. The following board checks were subsequently authorized and observed separately.

### Authorized hardware validation

**Pico 2 / handwritten Blinky.** The user approved starting hardware validation with handwritten Blinky on the non-wireless Pico 2 and confirmed BOOTSEL readiness with only that board connected. The agent verified the RP2350 boot volume and flashed `build/pico2/examples/blinky/qpcpp-blinky.uf2` with `cp -X`. The boot volume disconnected, `/dev/cu.usbmodem142101` appeared, and macOS reported USB product `Pico 2 Blinky`. The user confirmed the prompt, `Accepted` responses for `1`, `5`, and `9`, and corresponding 10%/50%/90% onboard LED duty cycles over a one-second cycle. This establishes basic GPIO LED and console operation by visual observation, not instrumented timing. The user also confirmed `0`, `10`, and `1,5` each produce `Invalid` and another prompt while preserving 90% duty cycle. The user confirmed terminal reconnect gives a fresh prompt while retaining 90%, and a normal USB power cycle restores 50%. Host inspection confirmed `/dev/cu.usbmodem142101` and USB product `Pico 2 Blinky` after the power cycle. Pico 2 / handwritten Blinky hardware smoke validation passed.

**Pico 2 / QM clone.** The user explicitly approved flashing the QM clone on the same non-wireless Pico 2 and confirmed BOOTSEL readiness. The agent verified the boot volume and copied `build/pico2/examples/blinky-qm/qpcpp-blinky-qm.uf2`. USB re-enumerated as `Pico 2 Blinky` on `/dev/cu.usbmodem142101`; after a brief mount-removal delay, the boot volume disappeared. The user confirmed the prompt, `Accepted` responses for `1`, `5`, and `9`, and the corresponding 10%/50%/90% LED on-time over a one-second cycle for the Pico 2 QM image. The user also confirmed `0`, `10`, and `1,5` each produce `Invalid` and another prompt while preserving 90% duty cycle on the Pico 2 QM image. The user confirmed a fresh prompt and retained 90% after terminal reopen, and 50% defaults after a normal USB power cycle. Host inspection again found `/dev/cu.usbmodem142101` and `Pico 2 Blinky` after reboot. Pico 2 / QM-modeled Blinky hardware smoke validation passed.

**Pico 2 W / handwritten Blinky.** The user approved switching to Pico 2 W for handwritten Blinky and confirmed BOOTSEL readiness with that board connected. The agent verified the boot volume and flashed `build/pico2_w/examples/blinky/qpcpp-blinky.uf2`. The boot volume disconnected, `/dev/cu.usbmodem142101` appeared and macOS reported `Pico 2 W Blinky`. The user confirmed the prompt, `Accepted` responses for `1`, `5`, and `9`, and the expected 10%/50%/90% duty cycles over a one-second cycle for Pico 2 W / handwritten Blinky. The user confirmed that `0`, `10`, and `1,5` each produce `Invalid` and another prompt while preserving 90% duty cycle. The user confirmed terminal reopen gives a fresh prompt with retained 90%, and a normal USB power cycle restores 50%. Host inspection again confirmed `/dev/cu.usbmodem142101` and `Pico 2 W Blinky` after reboot. Pico 2 W / handwritten Blinky hardware smoke regression passed.

**Pico 2 W / QM clone.** The user approved the final Pico 2 W QM regression flash and confirmed readiness. The agent verified the boot volume and copied `build/pico2_w/examples/blinky-qm/qpcpp-blinky-qm.uf2`. The volume disconnected and USB re-enumerated as `Pico 2 W Blinky` on `/dev/cu.usbmodem142101`. The user confirmed the prompt, `Accepted` responses for `1`, `5`, and `9`, and the corresponding duty cycles over a one-second cycle for Pico 2 W / QM. The user confirmed `0`, `10`, and `1,5` each produce `Invalid` and another prompt while preserving 90% duty cycle for Pico 2 W / QM. The user confirmed terminal reopen shows a fresh prompt while retaining 90%, and a normal USB power cycle restores 50%. Host inspection again confirmed `/dev/cu.usbmodem142101` and USB product `Pico 2 W Blinky` after reboot. Pico 2 W / QM hardware smoke regression passed.

All four board/example combinations passed the recorded smoke checks. Timing was visually observed, not instrument-measured; USB stress and fault injection were not performed on hardware. TASK-0020 is complete. The user then explicitly authorized committing the accumulated implementation, tests and documentation, including the QM-clone work.
