# Test Results

This document summarizes the verification performed on the final TM4C123 Smart Irrigation System.

## Functional Test Summary

| Test | Expected Result | Final Result |
|---|---|---|
| UART communication | Stable 115200 8N1 output | Passed |
| OLED initialization | SSD1306 starts and clears correctly | Passed |
| OLED live update | Sensor and pump values update continuously | Passed |
| DHT11 temperature | Valid temperature value | Passed |
| DHT11 humidity | Valid humidity value | Passed |
| DHT retry handling | Transient failed reads do not destroy last valid value | Passed |
| Soil ADC reading | Raw value changes with soil condition | Passed |
| Soil percentage conversion | Calibrated 0–100% interpretation | Passed |
| Soil dry test | Low percentage detected | Passed |
| Soil wet test | High percentage detected | Passed |
| LDR ADC reading | ADC value changes with illumination | Passed |
| Light percentage | Calibrated light response | Passed |
| ADC averaging | Reduced sample-to-sample fluctuation | Passed |
| Battery-pack measurement | Input supply measured before regulator setup | Passed: ~5.72 V |
| LM2596 output adjustment | Regulated output set before power-stage integration | Passed: 5.00 V |
| Relay driver | PB0 controls relay through BC337 stage | Passed |
| Relay default state | Pump path open when relay inactive | Passed |
| Pump operation | Pump runs when relay is activated | Passed |
| Pump stop | Pump stops when relay releases | Passed |
| Dry confirmation | Pump does not start from one isolated dry sample | Passed |
| Wet confirmation | Pump does not stop from one isolated wet spike | Passed |
| Hysteresis | Separate 35% / 50% thresholds prevent chatter | Passed |
| Safety timeout | Pump cannot run beyond 20 s | Passed |
| SW1 startup state | System powers up with pump disabled | Passed |
| SW1 first press | Automatic irrigation enabled | Passed |
| SW1 second press | Pump off and controller state reset | Passed |
| Motor EMI mitigation | Severe OLED corruption eliminated in final rebuild | Passed |
| Two-subsystem layout | Sensitive and power circuits operate together | Passed |
| Common ground reference | PB0 driver works with shared reference | Passed |
| Real soil irrigation | Soil probe responds during watering | Passed |

## LM2596 Supply Verification

The power-side supply was checked before the relay and pump stage was connected. A digital multimeter was used for both measurements.

1. The four-cell AA battery pack was measured at approximately **5.72 V**.
2. The battery pack was connected to the LM2596 input.
3. The multimeter was moved to the LM2596 output terminals.
4. The module's adjustment potentiometer was turned until the output reached **5.00 V**.
5. The 5.00 V output was rechecked before the relay and pump circuitry was integrated.

This removed the supply voltage as an unknown during the later relay and motor troubleshooting.

Evidence:

- [`images/power-subsystem/19-lm2596-output-adjustment-5v72.jpg`](../images/power-subsystem/19-lm2596-output-adjustment-5v72.jpg) — battery-pack measurement, approximately 5.72 V
- [`images/power-subsystem/20-lm2596-output-adjustment-5v00.jpg`](../images/power-subsystem/20-lm2596-output-adjustment-5v00.jpg) — regulated LM2596 output, 5.00 V

## Soil Moisture Calibration

Final calibration constants:

```c
#define SOIL_DRY_VALUE 4090U
#define SOIL_WET_VALUE 1650U
```

Expected conversion behavior:

```text
ADC >= 4090 -> 0%
ADC <= 1650 -> 100%
Intermediate values -> linear calibrated percentage
```

Screenshots from dry, wet, and intermediate measurements are stored under:

```text
images/soil-sensor/
```

## Irrigation Threshold Test

Final thresholds:

```text
Start region: Soil < 35%
Stop region:  Soil > 50%
```

The controller requires three consecutive measurements before accepting either transition.

### Expected Dry Sequence

```text
Sample 1 < 35% -> dry counter 1
Sample 2 < 35% -> dry counter 2
Sample 3 < 35% -> dry counter 3 -> pump may start
```

### Expected Wet Sequence

```text
Sample 1 > 50% -> wet counter 1
Sample 2 > 50% -> wet counter 2
Sample 3 > 50% -> wet counter 3 -> pump stops
```

This was introduced after motor-related ADC spikes showed that one sample was not sufficiently reliable for pump-state changes.

## Safety Timeout Test

Final maximum pump runtime:

```text
20 seconds
```

Expected behavior:

```text
Pump ON
...
Time:20s
Pump:SAFETY OFF
```

The safety state prevents the pump from running continuously when the wet threshold is not reached.

Representative UART evidence is stored in the troubleshooting/UART image directories.

## Relay Test

The relay was tested before full pump integration.

Verified electrical control path:

```text
PB0 -> 1 kΩ -> BC337 Base
Collector -> Relay IN
Emitter -> LM2596 OUT-
OUT+ -> 10 kΩ -> Relay IN
```

Observed verification indicators:

- relay module status LED changed state
- mechanical relay click occurred
- pump contact path opened/closed as expected

## Pump Power Test

Final relay contact wiring:

```text
OUT+ -> COM
NO -> Pump +
Pump - -> OUT-
```

The normally-open contact provides default pump-off behavior.

## EMI / Display Stability Test

### Earlier Failure

When the pump was added to the earlier shared breadboard layout, observed symptoms included:

- OLED corruption
- frozen OLED values
- DHT11 errors
- soil ADC spikes
- unstable pump decisions

### Final Mitigation

The final hardware includes:

```text
1N4007 flyback diode
100 nF motor capacitor
470 µF output capacitor
separate sensitive/power breadboards
controlled common-ground connection
```

### Final Result

The final rebuilt system showed stable display and control behavior compared with the earlier configuration.

## DHT11 Test

The final firmware uses retry logic and preserves the last valid DHT data.

During pump operation, new DHT reads are avoided to reduce interference with timing-sensitive communication.

Expected output after a valid reading:

```text
Temp:29C | Hum:32%
```

## OLED Test

Final interface:

```text
PB2 -> I2C0SCL
PB3 -> I2C0SDA
Address: 0x3C
```

Displayed fields:

```text
SOIL
TEMP
HUM
LIGHT
PUMP
TIME
```

Operating-state labels include:

```text
STOP
OFF
ON
SAFETY
```

## SW1 Test

LaunchPad SW1 / PF4 uses an internal pull-up.

Final state behavior:

```text
Power-up:
System STOP
Pump OFF

First press:
System RUN
Automatic control enabled

Second press:
System STOP
Pump OFF
Runtime reset
Safety reset
Dry/wet counters reset
```

## Final Integrated Test

The final integrated test verifies the complete signal path:

```text
Soil probe
-> ADC
-> filtering
-> calibrated moisture percentage
-> confirmation logic
-> irrigation controller
-> PB0
-> BC337
-> relay
-> pump
-> soil wetting
-> new soil measurement
```

At the same time:

```text
DHT11 -> temperature/humidity
LDR -> light percentage
OLED -> local live display
UART -> diagnostic output
SW1 -> operator START/STOP
```

The final prototype passed this integrated functional test.
