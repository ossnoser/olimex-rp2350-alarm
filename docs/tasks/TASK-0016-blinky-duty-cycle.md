# TASK-0016 - Blinky duty-cycle commands

## Status

Approved / completed; build, host tests, and hardware smoke validation passed.

## Objective

Replace off/on-second pairs with a single digit 1..9 selecting a 10%..90% LED-on duty cycle over a fixed one-second cycle.

## Individually approved decisions

1. Fixed one-second cycle: `1` is 100 ms on / 900 ms off, `5` is 500 ms on / 500 ms off, and `9` is 900 ms on / 100 ms off. Startup defaults to 50%, initially off.
2. Prompt `Enter duty cycle (1..9, 10%..90% on):` with CRLF. Accept exactly one digit 1..9, ignoring ordinary spaces. Retain `Accepted`/`Invalid` and repeated prompts, echo/editing, framing, overflow/control handling, and disconnect behavior. Apply accepted updates at the next LED transition without restarting the running phase. Invalid input preserves settings; latest accepted command wins. Reboot restores 50%.
3. Document and implement this scope, update static command events, host tests and documentation, build and test. Retain BSP separation and event-driven USB servicing. Flashing and committing require separate approval.

A phase spanning a command update retains its old duration; the newly entered phase uses the new duty cycle. The transition cycle can therefore span old/new settings. Once stable, off plus on is always one second.

## Planned changes

- Replace pair parser/result with a single duty-cycle value (tenths).
- Replace 25 pair events with nine immutable duty-cycle events; retain event precedence and cooperative scheduling.
- Derive on/off ticks from the fixed cycle and add a compile-time divisibility check for 10% resolution at the current tick rate.
- Update prompt and callback interfaces without changing the BSP/USB transport.
- Update parser/Console and real QP/QV timing tests, including all nine settings, input errors, phase preservation, event ordering, reconnect, startup, buffering and USB wakeups.
- Update the integration guide, task status and task index.

## Completion criteria

- Debug firmware build succeeds and ELF/UF2 are generated.
- All sanitizer-enabled host tests pass, including every duty cycle summing to one second.
- Old pair syntax, zero, multi-digit values, signs, decimals and non-space extra characters are rejected.
- Existing bounded Console and BSP/USB behavior is preserved.
- Hardware validation is reported separately and requires flashing approval. No unapproved commit.

## Result

Implemented the single-digit parser/result and prompt, nine immutable duty-cycle events, and on/off tick calculations over the fixed one-second cycle. Application callback names now explicitly refer to duty cycle. BSP/USB transport code is unchanged. Added compile-time checks for tenth-cycle resolution and sufficient Console response reservation for the new prompt.

Debug firmware build succeeded with the existing Pico 2 W configuration, producing `build/pico2_w/examples/blinky/qpcpp-blinky.{elf,uf2}`. All four AddressSanitizer/UndefinedBehaviorSanitizer-enabled host CTest cases passed. Parser/Console tests cover all nine settings, old pair rejection, zero/multi-digit/leading-zero and malformed input, framing, editing, bounds, buffer wrapping/partial writes, and reconnection. Real QP/QV tests exercise every command event and verify both settled phase lengths sum to 100 ticks (one second), current-phase preservation, command precedence/order, invalid input, retained settings, startup defaults, and USB wakeup behavior.

The integration guide documents the new protocol and preserves earlier milestones as historical records. The user subsequently authorized flashing by confirming board readiness. The agent verified the RP2350 BOOTSEL volume and copied the updated UF2 with `cp -X`; the volume disconnected and `/dev/cu.usbmodem143101` enumerated automatically. The user confirmed the new prompt and that `1`, `5`, and `9` each produce `Accepted` with the expected short/equal/long on durations and a steady one-second cycle after the current phase completes. Timing was visually observed, not instrument-measured. The user also confirmed `0`, `10`, and the old `1,5` syntax each produce `Invalid` and another prompt without changing the previously selected 90% duty cycle. The user confirmed that closing/reopening the terminal preserves 90% duty cycle with a fresh prompt, and a USB power cycle restores 50%. However, subsequent host checks found neither `/dev/cu.usbmodem*` nor `Pico 2 W Blinky` in the USB registry. Duty-cycle behavior checks passed by user observation. Subsequent USB-log inspection showed the upstream hub/adapter being power-cycled and then successful enumeration of `Pico 2 W Blinky`. A fresh host query found `/dev/cu.usbmodem142101` (different from the earlier path). No firmware change was made during this investigation. The earlier absence cannot be conclusively attributed to firmware from these observations; the user subsequently confirmed the prompt appears on the newly enumerated device. Hardware smoke validation is complete: representative 10%/50%/90% settings, invalid-input preservation, terminal reconnect retention, reboot defaults, and terminal operation after USB re-enumeration passed. Cadence was visually observed; exact phase lengths and all nine values were verified by host simulation rather than instrumented on the board. After hardware validation, the user explicitly authorized committing the completed changes.
