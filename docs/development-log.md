# Development Log

This log summarizes the engineering progression of the TM4C123 Smart Irrigation System from the initial monitoring prototype to the final stable irrigation controller.

## Phase 1 - Sensor and Display Foundation

The project began from a working TM4C123 environmental monitoring setup. The existing UART, DHT11, LDR, and SSD1306 OLED functionality provided the base for the irrigation controller.

The first irrigation-specific addition was the analog soil-moisture sensor on PE3/AIN0. Raw ADC values were observed through UART and converted into a calibrated moisture percentage.

Measured calibration values used in the final firmware:

```c
#define SOIL_DRY_VALUE 4090U
#define SOIL_WET_VALUE 1650U
```

## Phase 2 - Irrigation Control Logic

A simple threshold-based pump decision was considered first. The design was then improved with hysteresis:

```text
Below 35% -> dry region
Above 50% -> wet region
```

This separated the start and stop conditions and prevented repeated switching around one threshold.

A maximum pump runtime was also added as a safety feature.

## Phase 3 - Pump-Side Supply Setup

Before the relay and pump were integrated, the power-side supply was measured and adjusted.

The four-cell AA battery pack measured approximately **5.72 V** with a digital multimeter. The pack was connected to the LM2596 input, the multimeter was moved to the converter output, and the LM2596 adjustment potentiometer was turned until the output reached **5.00 V**.

This step was completed before relay and motor debugging so the power-stage supply was a known value rather than another unknown variable.

Evidence:

- `images/power-subsystem/19-lm2596-output-adjustment-5v72.jpg`
- `images/power-subsystem/20-lm2596-output-adjustment-5v00.jpg`

## Phase 4 - Relay Integration

The water pump required switching a separate power path. A relay module was introduced.

Direct GPIO control was not considered sufficiently reliable for the final relay-input arrangement, so a BC337 transistor interface was built.

Final driver topology:

```text
PB0 -> 1 kΩ -> Base
Collector -> Relay IN
Emitter -> LM2596 OUT-
LM2596 OUT+ -> 10 kΩ -> Relay IN
```

The relay was tested independently before the pump was connected.

## Phase 5 - Pump Power Path

The pump was wired through the relay's normally-open contact:

```text
LM2596 OUT+ -> COM
NO -> Pump +
Pump - -> LM2596 OUT-
```

Early wiring tests included silent-pump conditions until the relay contact path was corrected.

## Phase 6 - First Complete Automatic Irrigation Tests

The first integrated tests showed that the pump could start and stop, but the control behavior was not yet robust.

Observed behavior included:

- pump stopping after only a few seconds
- pump restarting shortly afterward
- runtime repeatedly returning to a lower value
- safety shutdown after repeated cycles
- large soil percentage jumps during pump operation

These observations showed that a single ADC result could not be trusted while the motor was active.

## Phase 7 - ADC Filtering and Confirmation Logic

ADC filtering was strengthened with 16-sample averaging.

The irrigation state transitions were then protected by confirmation counters:

```text
3 consecutive dry readings -> start permission
3 consecutive wet readings -> stop permission
```

This prevented isolated spikes from immediately changing the pump state.

## Phase 8 - Safety Timeout Refinement

A 10-second safety timeout was useful for early bench tests but too short for realistic soil wetting.

A 30-second value was considered longer than necessary for the prototype. The final value was set to 20 seconds.

```c
#define MAX_PUMP_RUNTIME 20U
```

The final interpretation is:

```text
Moisture target = normal stop
20-second limit = emergency protection
```

## Phase 9 - Motor-Induced Display and Sensor Problems

Once the pump was connected, the most difficult stage of the project began.

Observed failures included:

- OLED text corruption
- missing OLED characters/lines
- OLED freezing at an old soil value while UART continued updating
- DHT11 temperature errors
- soil ADC spikes
- pump-state instability

The fact that UART continued running during some OLED failures showed that the MCU had not completely crashed. The problem was localized to the sensitive peripheral/power environment.

## Phase 10 - Software Recovery Experiments

Firmware changes were tested to make DHT11 and OLED operation more fault tolerant.

Useful software changes included:

- DHT retries
- retaining the last valid DHT reading
- avoiding DHT reads while the pump was operating
- preserving ADC averaging
- confirmation counters

An aggressive I2C/OLED recovery attempt did not improve the system and was removed. This was an important diagnostic result: the root problem could not be solved purely in software.

## Phase 11 - Hardware EMI Mitigation

The pump path was modified with suppression and filtering components:

```text
1N4007 flyback diode
100 nF ceramic motor capacitor
470 µF LM2596 output capacitor
```

These changes reduced disturbances, but some OLED effects remained in the increasingly complex breadboard arrangement.

## Phase 12 - Grounding Investigation

Removing redundant ground paths improved OLED behavior. This indicated that return-current routing and common-ground layout were significant contributors.

The prototype had accumulated many jumper paths as components were added incrementally. At this stage it became more efficient to rebuild the hardware than to continue modifying the existing layout.

## Phase 13 - Two-Breadboard Rebuild

The system was rebuilt from scratch as two physically separated subsystems.

### Breadboard 1 - Sensitive Electronics

```text
Tiva C LaunchPad
SSD1306 OLED
DHT11
Soil sensor
LDR + 10 kΩ divider
```

### Breadboard 2 - Power / Pump Driver

```text
4 x AA battery pack
LM2596 adjusted to 5.00 V
BC337
1 kΩ
10 kΩ
Relay
Pump
1N4007
100 nF
470 µF
```

The two sides were connected only by the required PB0 control path and common ground reference.

This rebuild was the decisive hardware improvement. OLED stability and overall system behavior became significantly better.

## Phase 14 - Final Soil Test

The soil probe was inserted into real soil and the pump outlet was positioned away from the probe so the sensor would measure actual soil wetting rather than a direct water stream.

The intended final control sequence was verified:

```text
dry confirmation
-> pump ON
-> soil moisture rises
-> wet confirmation
-> pump OFF
```

The safety timeout remains available if the wet target is not reached within 20 seconds.

## Phase 15 - Demo Control with SW1

For a clean project demonstration, automatic irrigation was changed so it no longer starts immediately after power-up.

LaunchPad SW1 / PF4 became a user control:

```text
Power ON -> STOP mode
First SW1 press -> START automation
Second SW1 press -> STOP + reset
```

Sensors, OLED, and UART continue operating while the system is in STOP mode.

This allowed the final demonstration video to begin in a stable observable state and start irrigation only when commanded.

## Phase 16 - Final Documentation and Evidence

Hardware photos were organized into component, subsystem, troubleshooting, UART, and final-system categories.

The final documentation records both successful implementation and the engineering failures that shaped the design, especially:

- measured LM2596 supply setup
- relay-interface development
- pump contact wiring
- ADC instability
- DHT11 disturbance
- OLED/I2C corruption
- power filtering
- grounding redesign
- final subsystem separation

The final system is therefore documented as an engineering development process rather than only as a finished breadboard photograph.
