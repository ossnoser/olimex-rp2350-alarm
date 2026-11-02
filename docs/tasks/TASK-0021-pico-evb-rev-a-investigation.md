# TASK-0021 - PICO-EVB Rev. A integration investigation

## Status

Approved research/documentation / completed.

## Objective

Investigate use of a non-wireless Pico 2 / Olimex RP2350-PICO2-HDR mounted on Olimex PICO-EVB Rev. A before designing carrier-board firmware.

## Individually approved setup and scope

- Non-wireless Pico module and user-identified PICO-EVB Rev. A.
- Initial investigation/testing unloaded: relay contacts, optoisolated input terminals and external modules disconnected; no mains or controlled loads.
- Proposed sole power/programming/serial source is the Pico USB connector. EVB USB-C and power terminals remain disconnected; verify the power schematic before providing instructions.
- Research Rev. A power paths/jumpers/back-power risks, all four input/relay GPIO mappings and polarities, safe relay startup states, UEXT allocation/future MOD-RS485 conflicts, and a safe first-test procedure.
- Documentation only: no firmware/source changes, hardware energizing, flashing or commit authorized.

## Planned work

- Read official Olimex manual, Rev. A schematic and available source/demo files; record immutable upstream revisions where possible.
- Download research artifacts only under ignored project-local `build/pico-evb-research/`.
- Separate manual claims, schematic-derived facts, firmware implications and unresolved physical-board checks.
- Create `docs/hardware/pico-evb-rev-a-integration.md` and update hardware index/open points and task index.
- Preserve earlier hardware baseline as historical/contextual direction; do not silently convert marketing ratings into safe operating limits.

## Completion criteria

- Pin table and active/inactive levels supported by cited primary sources.
- Proposed single-USB power path checked against the schematic; unknowns and jumper conditions called out.
- Explain reset/boot versus firmware-controlled relay-off behavior and limitations.
- Map UEXT to Pico GPIO/UART/SPI/I2C and identify possible RS485 control-pin conflicts.
- Provide a gated low-voltage/unloaded first-test plan, not instructions for mains/load wiring.
- Document any necessary follow-up questions and implementation approvals; no unsupported safety guarantees.

## Result

Completed `docs/hardware/pico-evb-rev-a-integration.md` and updated the hardware documentation index/open points. Research used Olimex PICO-EVB commit `5a4360986a678dc177df4ea6674fc5769e91e64d`, including the 2026-05-15 Rev. A schematic/KiCad source, July 2026 manual and Arduino demo. Read the Pico 2 powerchain, TPS2116 datasheet, RP2350 E9/revision history and MOD-RS485 manual/Rev. A schematic as supporting primary sources.

Key findings:

- Inputs GP2/3/8/9 are active-low with external 47 kOhm pull-ups; relays GP10..GP13 are active-high, driven by biased NPN transistors.
- The proposed Pico-USB-only supply is supported by the manual and consistent with VSYS -> carrier L1 -> coil-rail routing. No carrier configuration jumper is needed. Carrier USB-C is power-only; rail voltage/current and physical assembly remain unmeasured.
- EXT1 GPIO23/24/25/29 positions are GND with a standard 40-pin Pico 2; the extra GPIO labels apply to PICO30. This does not invalidate the module's internal GP25 onboard LED.
- Current Blinky firmware does not explicitly initialize carrier relays, and its fail-stop handler would not automatically de-energize future latched relay outputs. A separate safe-off carrier firmware/policy is needed before the proposed powered test.
- RP2350-E9 affects A2 input-pad behavior and is fixed in A3/A4. Actual silicon revision and input thresholds remain to be checked; reset pull-down behavior must not be incorrectly conflated with the input-enabled E9 condition.
- UEXT uses UART0 GP0/GP1, I2C0 GP17/GP16 and SPI0 GP4/GP7/GP6/GP5. Default MOD-RS485 direction controls are DE GP6 and /RE GP5, conflicting with SPI clock/CS. Alternate I2C control positions encounter 2.2 kOhm pull-ups and can enable DE before firmware.
- Actual relay markings/coil budget need verification: schematic and manual photo name different parts. The earlier baseline's `15 V` versus vendor `15 A` description is explicitly flagged, not silently treated as a safe board load rating.

Validation was documentary: extracted/read manual text, rendered/inspected schematic and input/relay/layout figures, inspected KiCad component values/no configuration-jumper references, and cross-checked GPIO lists with the demo. Source hashes and revision-qualified links are in the report; external artifacts and the native PDF inspection helper remain under ignored `build/pico-evb-research/`. No firmware/build/dependency changes or physical measurements were performed.

The report provides a gated unloaded low-voltage first-test sequence and identifies remaining assembly, relay/silicon, power-budget, threshold and reset/fault checks. Research is complete; hardware correctness/safety is not certified. No power-on testing, flashing or commit is authorized by TASK-0021.
