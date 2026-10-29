# TASK-0015 - Interactive Blinky timing plan

## Status

Approved / completed; build, host tests, and hardware smoke validation passed.

The user approved each decision below, authorized the planning documentation, and subsequently approved implementation. Firmware changes and tests within this plan are authorized. Flashing was subsequently authorized for hardware validation, and after completion the user explicitly authorized committing the accumulated changes.

## Objective

Extend the standalone Pico 2 W Blinky example with an interactive USB serial console accepting off/on durations. Keep application behavior separate from board support and retain cooperative QV execution on core 0.

## Authorized changes for this planning step

- Create this document.
- Register TASK-0015 in `TASKS.md`.
- Preserve all existing work, including the uncommitted TASK-0014 implementation and validation documentation.

## Individually approved decisions

1. **Input syntax:** two whole-number values in the range 1..5, separated by exactly one comma. The first is seconds off; the second is seconds on. Ignore ordinary ASCII spaces anywhere. Reject decimals, signs, missing values, extra commas, and extra characters.
2. **Application time:** print `Accepted` after validation; accepted values affect the next LED state transition, not the running phase. The current phase completes on its existing timer.
3. **Defaults:** start at 0.5 seconds off / 0.5 seconds on until valid input is accepted. Invalid input leaves the current/pending timings unchanged.
4. **Protocol:** prompt with `Enter seconds off,on (1..5):` when a terminal connects and after every submitted line. Device messages end in CRLF. Respond with exactly `Accepted` or `Invalid`, followed by another prompt. Remove all existing `Blink!` output.
5. **Enter:** accept CR, LF, and CRLF; CRLF is one submission. Empty and spaces-only submissions are invalid.
6. **Echo:** device-side echo so typed input is visible in a normal Mac `screen` session without local echo.
7. **Editing:** Backspace (ASCII BS) and Delete (ASCII DEL) erase the preceding character and update the visible line; at the beginning of a line they do nothing. Spaces remain editable despite being ignored by the parser.
8. **Line bound:** at most 32 entered characters, including spaces and excluding Enter. Once exceeded, discard through Enter, respond `Invalid`, and prompt again. Do not accept truncated input or change timings.
9. **Disconnect/restart:** discard unfinished input and preserve accepted timings on terminal disconnection. Send a fresh prompt on reconnection. Reboot restores 0.5 s / 0.5 s defaults; no persistent storage.
10. **Active-object separation:** a Console active object polls USB every 20 ms without blocking, handles connection changes, echo/editing, and parsing, and sends timing-change events to Blinky. Blinky owns LED states/timing. USB operations are BSP-only; parsing never runs in interrupts.
11. **Output delivery:** fixed-size, nonblocking transmit buffering. Pause input consumption when output space is insufficient. Clear pending output on disconnection; reconnect with a fresh prompt. This replaces the earlier drop-on-timeout console-message policy.
12. **Repeated commands:** the latest accepted pair wins if several are accepted before a transition. Every valid submitted command still receives `Accepted`; never restart the current phase.
13. **Leading zeros:** after removing ordinary spaces, require exactly `[1-5],[1-5]`. For example `01,5` is invalid, while `1  ,  5` is valid.
14. **Unsupported controls:** tabs, arrow-key escape sequences, and other unsupported control characters invalidate the line. Discard through Enter, then respond `Invalid` and prompt. Enter and BS/DEL retain their specified special behavior before the line is invalidated.
15. **LED time events:** replace the periodic Blinky timer with a one-shot QP time event armed on entry to each LED state using that state's stored duration. Updating the pair must not rearm the running timer.
16. **Command-event storage:** use a fixed table of 25 immutable QP events, one per valid pair. No application heap allocation, dynamic event pool, or mutable shared command payload.
17. **Priorities:** Blinky priority 2, Console priority 1, cooperative QV on core 0. Core 1 remains unused. Console handlers must be short and bounded.
18. **Resource limits:** a 256-byte Console transmit buffer, at most 16 received bytes processed per 20 ms poll, at most 64 bytes offered for transmission per poll, and eight-entry queues for each active object. Reserve output capacity before consuming input so echo/responses cannot overflow the transmit buffer.
19. **BSP boundary:** expose terminal-connected status, nonblocking byte reads, nonblocking byte writes returning actual bytes accepted, and USB servicing as needed. Console owns line/transmit buffers and protocol handling. Ordinary printf-style return values are not sufficient for transmit accounting.
20. **Transport:** replace USB stdio with BSP-managed Pico SDK TinyUSB CDC. BSP owns initialization/servicing; application code never calls TinyUSB. It remains a USB serial device usable with Mac `screen`. UART and QS stay disabled.
21. **Source organization:** `blinky.cpp/.hpp` for LED behavior and timing-change events; `console.cpp/.hpp` for Console, buffers, echo/editing; `command_parser.cpp/.hpp` for hardware-independent validation; `main.cpp` starts both active objects. Keep parser independent of Pico SDK and QP.
22. **Validation:** Mac-hosted automated parser tests for all 25 pairs and invalid syntax; Console tests for line endings, editing, blank input, overflow, and controls; board tests for prompts/responses, asymmetric timings, update boundaries, latest-command-wins, and reconnection. Report automated and user-observed results separately.
23. **Boundary ordering:** accepted timing-change events take precedence over a timeout already queued but not dispatched. Finish at most one submitted command per Console handler, then yield to Blinky. This preserves command order and lets the next entered LED state use the accepted values.
24. **Documentation gate:** record the complete plan and task index now; obtain separate implementation approval before firmware changes. No commit or flashing authorized by this gate.

## Protocol examples

The following includes device echo of user input; each completed line is followed by CRLF:

```text
Enter seconds off,on (1..5):
2,3
Accepted
Enter seconds off,on (1..5):
0,6
Invalid
Enter seconds off,on (1..5):
```

`2,3` means two seconds off and three seconds on. It does not restart the phase in progress. No unsolicited `Blink!` lines remain.

25. **Enumeration follow-up:** after a second normal reboot showed default blinking but no USB enumeration, the user approved supplementing the 20 ms poll with event-driven USB servicing. TinyUSB's event hook notifies Console via a coalesced QP event; Console services the stack promptly in application context. Parsing remains on the bounded 20 ms poll; framework tick, active-object priorities, and LED timing remain unchanged. Delayed USB service is a working hypothesis, not an established root cause. Reflashing still requires approval.

## Implementation scope (subsequently authorized)

### Application

- Add the hardware-independent command parser with an explicit result and off/on values.
- Add a statically allocated Console active object, poll time event, eight-entry queue, 32-character line storage plus any required terminator, and 256-byte transmit buffer.
- Keep line-ending/editing/overflow state separate from the pure numeric grammar parser.
- Bound receive/transmit work per handler; stop consuming at a submitted line and leave subsequent bytes for later processing. Do not drop unread bytes fetched beyond that line.
- Reserve enough transmit capacity for echo/edit operations, a complete response, and the next prompt before consuming input that could generate them. Transmit only the byte count accepted by the transport; retain unsent bytes.
- Normalize submitted-line echo so the response starts on a new line and CRLF input cannot trigger a second response.
- Discard input/output session state on disconnect; ensure a reconnect cannot submit a stale partial command or replay stale application output.
- Store Blinky off/on durations as ticks, preserving the approved 100 Hz tick and 50-tick initial phase. Convert accepted seconds to 100..500 ticks.
- Replace the periodic LED event with one-shot arming in state-entry actions.
- Implement immutable timing-change events and precedence over queued timeout events. Process only one completed command per Console handler and yield before the next command.
- Increase QP supported active objects to two and start both queues/objects before the framework startup hook enables ticking.
- Keep fail-stop handling and assertions enabled.

### BSP/build

- Replace the line-only USB output API with a small transport interface using actual byte counts. Exact signatures are implementation details constrained by the approved boundary.
- Initialize TinyUSB CDC without waiting for a terminal; service it in application context via the BSP. USB hardware IRQ processing remains transport-only.
- Supply required TinyUSB configuration/descriptors and build dependencies; remove USB stdio ownership to avoid competing USB stacks/tasks. Preserve third-party notices if reusing SDK material.
- Keep UART and QS disabled, board/platform guards intact, Debug/deoptimized build settings, and the approved QP/C++ pin.
- Keep memory use fixed and handlers bounded. Do not claim USB-stack execution is hard real-time without measurement.
- Add host-test build/run support independent of the embedded cross-compilation configuration.

### Documentation

- Update the integration guide for the new protocol and the replacement of TASK-0014's output-only behavior.
- Document Mac connection, input examples, CRLF/echo behavior, buffer bounds, timing semantics, and actual test results.
- Record implementation approval and progress before source edits; update the task index/status as work proceeds.

## Validation matrix

### Parser tests on the Mac

- All 25 combinations from `1,1` through `5,5`.
- Spaces before/after/between grammar characters.
- `0,6`, `7,8`, `6,1`, `1,0`, `01,5`, signs, decimals, missing/extra delimiters, missing values, letters, and empty/spaces-only lines.
- Confirm the parser does not silently accept prefixes or ignore non-space characters.

### Console/ordering tests

- CR, LF, CRLF, blank submissions, repeated submissions, and prompt/response counts.
- BS/DEL at start, normal erasure, and editing spaces.
- Exactly 32 characters versus overflow; overflow/control-invalid lines stay invalid through Enter even if later keys resemble corrections.
- Tabs and escape sequences; recovery on the next line.
- Partial USB writes, full transmit buffering, input pausing/resumption, and per-handler byte limits.
- Multiple complete commands in a receive stream; at most one submission handled before yielding, with remaining input retained.
- Disconnect discards session input/output while preserving LED settings; reconnect prompts once for the new session.
- Accepted timing events precede queued timeouts; later accepted commands supersede earlier values without rearming the current phase.

### Board tests (separate flashing approval required)

- Boot with no terminal and confirm default blinking proceeds.
- Connect from macOS using the actual `/dev/cu.usbmodem*` path with `screen ... 115200`; no local echo required.
- Confirm prompt and visible/editable input; no `Blink!` output.
- Check valid/invalid responses and repeated prompts.
- Use `1,5` and `5,1` to confirm off/on order and asymmetric durations.
- Change values during a phase and confirm the current phase completes before new timing applies.
- Submit several commands quickly and confirm the latest accepted pair governs subsequent phases.
- Close/reopen the terminal with partial input and pending output; settings persist and a fresh prompt appears without stale commands/output.
- Reboot and confirm defaults return.
- Distinguish visual cadence observations from instrument-measured timing and simulated backpressure tests from actual USB stress testing.

## Implementation completion criteria

- Both embedded Debug and host-test builds succeed and test results are recorded.
- The input/response protocol and error cases match all approved decisions.
- Timing changes are event-driven, phase-preserving, and ordered correctly at timeout boundaries.
- Application files have no Pico SDK/TinyUSB implementation dependencies; BSP is separate.
- Fixed memory and per-handler budgets are respected; serial flow control does not block QV execution waiting for host output.
- Authorized hardware validation is recorded honestly, with unperformed cases left pending.
- Updated Mac instructions and implementation/test documentation exist.
- No unapproved flashing, external-source modifications, or Git commit.

## Result

Implemented the approved Console/Blinky separation and TinyUSB transport. Added a pure command parser and a hardware-independent Console session core for host testing; `console.cpp` adapts that core to the BSP and QP active object. Updated QP configuration for two active objects, with Blinky at priority 2 and Console at priority 1. Timing commands use 25 immutable events posted LIFO, and Console yields after one submission. LED phases use one-shot time events and no longer emit `Blink!`.

BSP owns CDC descriptors, nonblocking TinyUSB servicing, actual-byte-count reads/writes, and terminal-session generations. Session changes clear transport/session buffers; generation tracking also detects close/reopen callbacks processed within one Console poll. USB stdio and UART stdio are not compiled. The CDC development VID/PID matches the Pico SDK defaults; this is not a product-specific USB identifier allocation.

Validation performed:

- Embedded Debug configure/build and clean-first rebuild succeeded with Pico SDK 2.3.0, GNU Arm 14.2.1, and `PICO_DEOPTIMIZED_DEBUG=1`.
- Updated ELF/UF2 generated at `build/pico2_w/examples/blinky/qpcpp-blinky.{elf,uf2}`.
- Checked actual build commands and ELF symbols: TinyUSB device stack, descriptors, connection callbacks, and both active objects are present; USB stdio/UART stdio sources and the USB stdio initializer are absent.
- Configured and built `tests/host` using native AppleClang with AddressSanitizer and UndefinedBehaviorSanitizer enabled. All three CTest cases passed: `parser_console`, `qp_timing_ordering`, and `startup_without_terminal`.
- Parser/Console tests cover all 25 valid pairs, invalid grammar, unchanged result on parse failure, CR/LF/CRLF including split CRLF, editing, exact line bound/overflow, unsupported controls, invalid-line recovery, queued commands, receive/transmit budgets, blocked/partial writes with ring-buffer wrapping, and session changes without stale input/output.
- Timing tests compile the production Blinky and Console against the real upstream QP core/QV scheduler with a single-threaded host port shim and mock BSP. They verify default phases, startup without a terminal, accepted updates without phase restart, precedence over queued timeouts, latest-command-wins across Console yields, invalid commands preserving settings, and settings surviving terminal reconnect.
- `git diff --check` passed.

Limitations: host tests do not validate the Arm critical-section port, USB descriptors/enumeration, DTR behavior on macOS, electrical disconnection, or physical LED timing. USB servicing is nonwaiting but TinyUSB drains its event queue internally; no hard execution-time bound or hostile-USB-flood guarantee is claimed. Finite device/host buffers cannot guarantee lossless arbitrarily large pasted input if the host ignores transport backpressure.

The user subsequently put the board into BOOTSEL mode in response to the flashing request. The agent verified `INFO_UF2.TXT`, copied the updated UF2 with macOS extended-attribute copying disabled, and observed the boot volume disconnect. USB CDC did not enumerate during the subsequent checks. The user confirmed default LED blinking continues, establishing application/tick activity but not USB functionality. After the user unplugged/reconnected USB normally (without BOOTSEL), macOS enumerated `Pico 2 W Blinky` at `/dev/cu.usbmodem141101`. Normal reconnect enumeration is confirmed; the initial post-UF2 enumeration failure remains unexplained and repeatability is not established. The user opened `screen` on the detected serial device and confirmed the prompt, visible `1,5` input, `Accepted` followed by another prompt, and 1-second-off / 5-second-on timing after the current phase completed. The user also confirmed that `0,6` and `7,8` each produce `Invalid` and another prompt while retaining 1-second-off / 5-second-on timing. The user confirmed editing `2,4` to `2,3` with Backspace/Delete, receiving `Accepted` and another prompt, and observing 2-second-off / 3-second-on timing after the current phase completed. The user confirmed terminal-close/reopen behavior: partial `4,` input was discarded, a fresh prompt appeared, 2-second-off / 3-second-on settings persisted, and `5` alone produced `Invalid` rather than completing the old command. Because Ctrl-A did not work in the user's terminal setup, they closed the specific session from another Terminal with `screen -ls` and `screen -S SESSION_NAME -X quit`. The user also confirmed that ` 3 , 1 ` is accepted with 3-second-off / 1-second-on timing, while `01,5` and an empty line each produce `Invalid` and a new prompt without changing those timings. The user confirmed that a 40-character line and an input containing Tab each produce `Invalid`, followed by successful recovery with `2,1` and 2-second-off / 1-second-on timing. The user confirmed that pasting `5,5`, `4,3`, and `1,2` yields three `Accepted` responses/prompts and the final 1-second-off / 2-second-on timing after the current phase completes. The user confirmed reboot restores the 0.5-second defaults without a terminal, but USB failed to enumerate on that reboot. The host confirmed no serial device was present. Startup USB reliability was therefore a hardware validation blocker for the polling-only build; see the follow-up results below. Explicit line-ending variants remain tested on the host rather than separately exercised on hardware. Following separately approved decision 25, implemented an atomic, lock-free wakeup callback in the BSP and TinyUSB event hook, with a coalesced Console USB-service event. An initial kick covers USB events received before callback registration. Servicing clears the coalescing flag before draining USB work so an interrupt arriving during service can queue one follow-up event. USB-service events do not run the parser or bypass the 20 ms input budget.

The updated firmware built successfully. All four sanitizer-enabled host CTest cases passed, including the new `usb_service_wakeup` case: 100 notifications coalesce into one service dispatch, work arriving during service triggers a follow-up, input remains untouched outside its poll, and LED tick timing remains unchanged. ELF inspection confirms a strong `tud_event_hook_cb` implementation is linked. The user subsequently authorized reflashing by confirming BOOTSEL readiness. The agent verified the boot volume and copied the revised UF2 with `cp -X`. The boot volume disconnected and `/dev/cu.usbmodem141101` enumerated automatically within the 15-second check, without an extra USB reconnect. This is one successful post-flash enumeration, not yet proof of reliable reboot enumeration. The user confirmed the revised build shows the prompt, accepts `1,5` with the expected timing, and rejects `0,6` without changing timings. The user confirmed a subsequent normal USB power cycle restored the 0.5-second default cadence. Host inspection confirmed `Pico 2 W Blinky` and `/dev/cu.usbmodem141101` enumerated on that cold start. The user repeated a normal unplug/reconnect once more and confirmed the prompt appeared again. Host inspection again found `/dev/cu.usbmodem141101`. The event-driven follow-up has now passed one immediate post-flash enumeration and two normal cold-start checks. The earlier failure has not recurred in those checks; this is a successful smoke test, not an endurance or exhaustive root-cause proof.

TASK-0015 is completed with build, automated-test, and user-confirmed hardware smoke validation. Explicit CR/LF/CRLF variants, partial-write/backpressure behavior and the exact queued-timeout race were tested on the host, not instrumented on the board. Broad input/edit/reconnect tests were performed on the first interactive build; prompt/accept/reject/timing and repeated cold starts were rechecked after the USB-service follow-up. After hardware validation, the user explicitly authorized committing this task's implementation, tests, and documentation together with the accumulated TASK-0014 documentation. See the integration guide for host-test commands and the board validation checklist.
