# TASK-0014 - Blinky USB serial output

## Status

Approved / completed.

## Objective

Print `Blink!` on USB serial whenever Blinky enters its LED-on state, while keeping USB implementation inside the BSP.

## Individually approved design

1. Add `BSP::writeLine("Blink!")`, called immediately after turning the LED on. No SDK/USB implementation in application code.
2. Never wait for a terminal at startup; blink immediately and do not replay messages missed while disconnected.
3. Best-effort output: discard output under backpressure/disconnection rather than wait for USB buffer space. Preserve cooperative application responsiveness; no guaranteed delivery.
4. Output `Blink!\r\n` only, without timestamps or additional messages.
5. Implement and document this task, build/test, keep UART and QS disabled, and provide macOS `screen` connection instructions. Flashing and committing require separate approval.

## Planned changes

- Extend the BSP header and Pico 2 W implementation with USB CDC initialization and line output.
- Enable USB stdio consistently in the BSP/executable build; disable connection waiting, buffer-space waiting, and stdio lock waiting. Avoid explicit flush calls that can wait for output completion.
- Call the BSP line-output function on LED-on state entry.
- Update the integration guide and task index with build, connection, and validation instructions/results.

## Completion criteria

- Debug build produces ELF/UF2 with USB CDC enabled and UART/QS disabled.
- Static review confirms no startup connection wait, no deferred/replay queue, and zero output wait timeout.
- On an explicitly authorized hardware test, the user sees one `Blink!` line per LED-on transition in a Mac terminal, while blinking continues when disconnected.
- Report build-only validation separately from hardware validation; do not claim unperformed backpressure/timing tests.
- New code retains GPL-3.0-or-later identifiers; no outside-project source changes or unapproved commit/flashing.

## Result

Implemented USB CDC initialization and `BSP::writeLine()`, called immediately after LED-on. UART/QS remain disabled. USB connection wait, buffer-space timeout, and stdio lock timeout are zero; raw writes supply explicit CRLF without implicit flushing or a replay queue.

Debug configure/build succeeded with SDK 2.3.0 and `PICO_DEOPTIMIZED_DEBUG=1`, producing updated ELF/UF2 artifacts at `build/pico2_w/examples/blinky/qpcpp-blinky.{elf,uf2}`. Checked actual SDK/BSP compilation commands for consistent USB-only and zero-wait configuration. ELF symbol inspection confirmed USB CDC and the BSP output function are linked. `git diff --check` passed. The integration guide includes Mac `screen` instructions and validation steps.

Status: build-verified and hardware smoke-test validated. The user opened the Mac terminal and confirmed `Blink!` output with continued LED blinking, then confirmed that blinking continued after closing the terminal and output resumed once per second without a backlog burst after reopening it. Timing was visually observed, not instrument-measured; USB saturation/backpressure was not stress-tested. Best effort may drop partial lines under congestion; USB processing still has finite execution cost and is not claimed hard real-time. The user subsequently indicated the board was in BOOTSEL mode in response to the flashing approval request. The agent verified the RP2350 boot volume and copied the updated UF2. macOS `cp` reported an extended-attribute copy error; the boot volume nevertheless disconnected and `/dev/cu.usbmodem144101` appeared after reboot, consistent with the updated USB CDC firmware running. The user subsequently confirmed terminal output, LED blinking, continued blinking with the terminal closed, and normal output resumption without a backlog burst after reopening the terminal. After TASK-0015 hardware validation, the user authorized committing the accumulated changes, including this task's documentation.
