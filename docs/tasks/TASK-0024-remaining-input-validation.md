# TASK-0024 - Remaining input validation

## Status and authorization context

Completed for digital functional validation; IN2..IN4 analog margins remain unmeasured.

After completion/cleanup of TASK-0023, the agent proposed testing IN2, IN3 and IN4 individually using the same 5 V DC procedure, without firmware changes or relay actuation. The user responded by reporting that they had checked all inputs and supplied the trace below. This records the user's independently performed tests and accepts their response as continuation/results-recording consent; it does not claim the agent directed or independently observed the actual wiring/power sequence.

No source changes, flashing, commands to energize relays, mains/load wiring, installations or commits are authorized. Additional physical steps remain one at a time.

## Objective and scope

Record/review each input's isolated HIGH -> LOW -> HIGH response with existing firmware; obtain available analog GPIO and relay-indicator observations, distinguish digital functionality from electrical qualification, and finish with field power removed and safe wiring cleanup.

The proposed conditions were the TASK-0023 fixed 5 V DC source (0.5 A maximum capacity, not precision current limiting), direct to the selected protected input's DC/COM terminals using onboard series resistance, with no extra resistor. Pico USB remains carrier power; contacts/other modules remain disconnected; cold wiring before moving channels. The user has not separately detailed the physical sequence/source settings for this new trace, so do not invent such evidence.

## References and mapping

- [TASK-0023](TASK-0023-in1-low-voltage-input-test.md): IN1 nominal 5 V pilot and measurement conventions.
- [TASK-0022](TASK-0022-pico-evb-diagnostic-plan.md): existing diagnostic, sampled/latest-value logging, safe relay-off initialization and fault action.
- [Rev. A source investigation](../hardware/pico-evb-rev-a-integration.md): protected DC input/COM routing, GPIO labels versus physical numbering, and A2/E9 caveats.

| Input | GPIO label used for analog measurement |
|---|---|
| IN1 | GP2 |
| IN2 | GP3 |
| IN3 | GP8 |
| IN4 | GP9 |

Measure GPIO voltages against Pico/system GND, not isolated field COM. No 5 V connection to GPIO/3V3. Keep relay contacts empty. Do not bypass components or change pulls/voltage to force a LOW.

## User-supplied trace

```text
Pico 2 EVB Diagnostic / configured PICO-EVB Rev. A
RP2350 raw revision=2 (A2)
WARNING: GPIO E9 applies; validate input voltage margins.
Relay commands: OFF OFF OFF OFF (no contact feedback)
Inputs: HIGH=inactive, LOW=active; raw/unfiltered
t=3600ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
t=46900ms IN1=LOW IN2=HIGH IN3=HIGH IN4=HIGH
t=47860ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
t=61060ms IN1=HIGH IN2=LOW IN3=HIGH IN4=HIGH
t=62180ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
t=74460ms IN1=HIGH IN2=HIGH IN3=LOW IN4=HIGH
t=75240ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
t=80980ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=LOW
t=81660ms IN1=HIGH IN2=HIGH IN3=HIGH IN4=HIGH
```

## Review to date

All four channels individually transitioned LOW, then recovered HIGH; other reported channels stayed HIGH. Initial and final snapshots were all HIGH. These establish the expected digital mapping/polarity in the provided observations. They do not prove continuous absence of other pulses, physical switching latency, debounce performance, transient immunity or analog margins. Logging is sampled and may coalesce updates; manual switching timestamps are not precision timing measurements.

The banner reports commanded relay states, not contact/coil feedback. The user subsequently stated they did not measure active GPIO voltages for IN2..IN4. These readings remain unperformed, not presumed 0 V from the digital LOW labels. The user confirmed all four relay indicators stayed OFF throughout the tests. Physical cleanup/power state has not yet been confirmed. IN1's analog observations from the prior pilot remain separate evidence; do not generalize them automatically to the other optocouplers or all A2 units.

## Planned project changes

This task/index and relevant hardware/example result documentation only. No production/model/generated/BSP source changes. Any agent-created raw artifacts remain inside ignored project-local `build/`; this trace was pasted by the user, not captured by the agent.

## Completion criteria and limits

- Record each channel's digital response and available analog/physical observations with clear provenance.
- Obtain active GPIO voltages where available or explicitly identify unperformed measurements; do not call unmeasured margins qualified.
- Confirm relay indicators stayed OFF and no relay actuation/load tests occurred.
- Confirm field power removed and safe power-off wiring cleanup.
- Record unresolved electrical/threshold/noise/fault cases without overstating the digital pass or authorizing additional hardware/software behavior.

## Next checkpoint

The user was instructed to switch OFF/disconnect the field supply, unplug Pico USB before handling wiring, and remove all input test wires. They confirmed cleanup done: board user-reported unpowered, every terminal unwired.

TASK-0024 is complete as a digital functional test record. All four inputs individually went HIGH -> LOW -> HIGH with other reported channels HIGH; all relay indicators stayed OFF according to the user. IN2..IN4 active analog levels were explicitly not measured. No firmware changes, flashing, relay actuation commands, process interruption or commits occurred.

Further analog tests or relay actuation need separate approval. Before planning relay energization, identify the actual fitted relay markings/coil requirements and review the unmeasured coil supply/current budget. Existing OFF commands/LED observations do not establish contact/load safety.
