# Troubleshooting

This document records the major problems encountered during development of the TM4C123 Smart Irrigation System, the observed symptoms, the diagnostic reasoning, the implemented corrections, and the final verification results.

The project evolved from a basic sensor/display prototype into a two-subsystem irrigation controller with a relay-driven pump. Most of the difficult failures appeared only after the pump and power electronics were integrated, because the motor introduced current transients and electrical noise into a circuit that also contained ADC, I2C, UART, and timing-sensitive digital sensor interfaces.

---

## 1. Soil Moisture Calibration Was Not Initially Meaningful

### Symptom

Raw ADC values were available, but the displayed soil percentage did not yet correspond to a useful dry/wet interpretation.

### Cause

The sensor required empirical calibration. A generic 0–4095 mapping would not represent the actual dry and wet endpoints of the physical sensor and probe.

### Solution

Measured dry and wet limits were used:

```c
#define SOIL_DRY_VALUE 4090U
#define SOIL_WET_VALUE 1650U
```

The percentage function clamps values outside the calibrated range and maps the interval to 0–100%.

### Verification

The sensor was tested in dry, wet, and intermediate conditions while UART data was monitored.

---

## 2. A Single Moisture Threshold Caused Unstable Pump Logic

### Symptom

A single threshold made it possible for the pump state to change repeatedly if the measured moisture hovered near the threshold.

### Cause

No hysteresis was present.

### Solution

Two separate thresholds were introduced:

```text
Soil < 35%  -> irrigation request
Soil > 50%  -> irrigation stop request
```

This creates a 15 percentage-point hysteresis band.

### Verification

The pump no longer switched rapidly around one threshold.

---

## 3. The Relay Could Not Be Reliably Driven Directly from the Tiva GPIO

### Symptom

The relay did not behave reliably when controlled directly from the TM4C123 GPIO output.

### Cause

The Tiva GPIO operates at 3.3 V logic, while the relay input stage was powered from the LM2596 output and did not provide a clean direct interface for the 3.3 V control signal.

### Solution

A BC337 transistor interface was introduced. The final topology used:

```text
PB0 -> 1 kΩ -> BC337 Base
BC337 Collector -> Relay IN
BC337 Emitter -> LM2596 OUT-
LM2596 OUT+ -> 10 kΩ -> Relay IN
```

The 10 kΩ resistor acts as a pull-up on Relay IN, while the BC337 provides a controlled low-side path.

### Verification

Relay LED and mechanical switching behavior matched the software control state.

---

## 4. BC337 Driver Wiring Required Rework

### Symptom

During early tests the relay driver did not switch as expected, and transistor heating was observed during incorrect configurations.

### Cause

The transistor stage was still being assembled and the base, collector, emitter, resistor, and relay-input nodes were not initially wired in the final verified topology.

### Solution

The driver was rebuilt around the verified node structure:

```text
PB0 -> 1 kΩ -> Base
Collector -> Relay IN
Emitter -> common power ground
OUT+ -> 10 kΩ -> Collector / Relay IN node
```

The transistor pin orientation was checked before power was reapplied.

### Verification

The relay could then be switched repeatedly without abnormal transistor behavior.

---

## 5. Pump Was Connected to the Wrong Relay Contact Path During Early Tests

### Symptom

The relay clicked, but the pump remained silent or behaved differently from the intended normally-off logic.

### Cause

The relay's COM/NO contact path had not yet been wired as the final pump power path.

### Solution

The final connection became:

```text
LM2596 OUT+ -> Relay COM
Relay NO    -> Pump +
Pump -      -> LM2596 OUT-
```

The normally-open contact was deliberately selected so the pump is physically disconnected from the supply when the relay is not energized.

### Verification

The pump began operating when the relay switched to the active state and stopped when the relay released.

---

## 6. Pump Safety Logic Initially Behaved Like a Normal Stop Condition

### Symptom

The pump reached the safety timeout and stopped, even though the intended normal behavior was to continue only until the soil reached the wet threshold.

### Cause

The timeout had initially become too prominent in the control logic. The design goal was clarified: moisture is the normal stop condition; runtime is only a fail-safe limit.

### Solution

The algorithm was structured so that:

```text
Normal stop  -> confirmed soil > 50%
Emergency stop -> maximum runtime reached
```

### Verification

UART output clearly differentiated normal OFF behavior from `SAFETY OFF`.

---

## 7. Initial Safety Timeout of 10 Seconds Was Too Short

### Symptom

The pump could enter safety shutdown before the soil had enough time to reach the target moisture level.

### Cause

The original 10-second value was useful for bench testing but too restrictive for realistic irrigation.

### Solution

Several candidate values were considered. 30 seconds was considered unnecessarily long for the prototype, and the final value was set to:

```c
#define MAX_PUMP_RUNTIME 20U
```

### Verification

The system retained protection against a continuously running pump while allowing more realistic watering time.

---

## 8. Pump Restarted or Stopped Because of Transient Soil Readings

### Symptom

UART logs showed that the pump could run for several seconds, stop, restart, and later enter safety mode. Soil readings sometimes jumped abruptly, including values close to 0% and 100% within a short interval.

### Cause

Motor operation introduced electrical disturbances into the analog measurement path. A single noisy reading could incorrectly satisfy the control condition.

### Solution

Two software filtering mechanisms were introduced:

1. 16-sample ADC averaging.
2. Consecutive-measurement confirmation counters.

Final confirmation logic:

```text
3 consecutive Soil < 35% -> allow pump start
3 consecutive Soil > 50% -> normal pump stop
```

### Verification

Single-sample spikes no longer directly changed the pump state.

---

## 9. OLED Display Became Corrupted When the Pump Operated

### Symptom

The SSD1306 display showed corrupted text, missing characters, partial lines, or visually damaged content shortly after pump activation.

### Cause

The pump is an inductive, high-current load. Motor startup and switching produced electrical noise and supply disturbances that coupled into the sensitive OLED/I2C circuitry.

### Solution

The final correction required both hardware suppression and physical layout changes:

- 1N4007 flyback diode on the pump power path
- 100 nF ceramic capacitor across the motor terminals
- 470 µF electrolytic capacitor across LM2596 OUT+ and OUT-
- separation of sensitive electronics and pump/power electronics onto two breadboards
- simplified ground interconnection

### Verification

After the final two-breadboard rebuild, the OLED remained stable during normal system operation.

---

## 10. DHT11 Produced `TEMP: ERR` During Pump Operation

### Symptom

The display/UART occasionally reported a temperature error while the pump was switching or running.

### Cause

DHT11 communication depends on microsecond-scale pulse timing and is vulnerable to disturbed supply/reference conditions and blocking timing interactions.

### Solution

The firmware was changed so that:

- DHT11 reads are retried.
- the most recent valid temperature/humidity values are retained.
- DHT11 polling is avoided while the pump is actively running.

### Verification

The final system preserved valid environmental values more reliably instead of replacing the display with transient error data.

---

## 11. OLED Display Froze While UART Continued Updating

### Symptom

A critical test showed UART reporting a changing soil value while the OLED remained stuck at an old value such as 0%.

### Cause

The microcontroller and ADC control loop were still running, but the OLED/I2C subsystem had stopped updating correctly. This separated the failure from a complete firmware crash and pointed toward display/I2C disturbance.

### Solution

Several software recovery ideas were evaluated, but the most effective final solution was hardware/layout-oriented rather than repeatedly resetting I2C:

- power filtering
- motor suppression
- physical subsystem separation
- simplified ground path

### Verification

The final rebuilt layout restored continuous OLED updates.

---

## 12. Automatic I2C Recovery Attempt Did Not Improve the System

### Symptom

A software recovery modification intended to restore the OLED after an I2C disturbance caused the display update behavior to become worse or stop entirely.

### Cause

The underlying problem was primarily electrical. Adding aggressive software recovery increased complexity without removing the physical disturbance source.

### Solution

The recovery experiment was removed and the code returned to the previously working I2C implementation. The real fix focused on the power and grounding architecture.

### Verification

The simpler I2C implementation worked correctly after the hardware rebuild.

---

## 13. Multiple OLED Characters or Fields Appeared Incorrect

### Symptom

Some OLED text looked incomplete or unexpected during intermediate firmware revisions.

### Cause

The custom 5x7 font contained only a subset of characters required by the current user interface. Some newly introduced text used characters that were not defined.

### Solution

The display strings were restricted to supported characters or the required glyphs were added to the font table.

### Verification

Final labels such as `SOIL`, `TEMP`, `HUM`, `LIGHT`, `PUMP`, `STOP`, `OFF`, `ON`, `SAFETY`, and `TIME` displayed correctly.

---

## 14. Source File Was Accidentally Duplicated Inside the Same File

### Symptom

Code Composer Studio reported many errors such as:

```text
function "UART_SendText" has already been defined
function "UART_SendNumber" has already been defined
function "OLED_Init" has already been defined
function "DHT11_Read" has already been defined
function "main" has already been defined
```

The source file had grown to more than two thousand lines.

### Cause

A complete revised code block had been pasted below an older complete implementation instead of replacing it.

### Solution

The duplicate section was removed and the source was rebuilt as one complete implementation containing exactly one definition of every function and one `main()`.

### Verification

The duplicate-definition build errors disappeared.

---

## 15. Compile Errors Cascaded After an Earlier Syntax Error

### Symptom

CCS displayed a large number of errors, including parser restart messages and repeated function-definition errors.

### Cause

A syntax problem earlier in the source caused the compiler parser to lose context. The duplicated source code then produced additional real definition errors.

### Solution

The code was cleaned from the top down rather than addressing each later error individually. The earlier syntax issue and duplicated implementation were removed first.

### Verification

The project returned to a normal build state.

---

## 16. Common-Ground Topology Became Too Complex

### Symptom

The OLED improved when unnecessary ground connections around the breadboard were removed, indicating that the physical return-current layout was affecting the sensitive electronics.

### Cause

The evolving prototype accumulated multiple power and ground paths. Pump current and digital/analog return currents were sharing breadboard paths in an uncontrolled way.

### Solution

The final architecture used two physically separated breadboards and a deliberate single common reference connection between the sensitive and power sides.

### Verification

OLED stability improved significantly after the rebuild.

---

## 17. Final Hardware Was Rebuilt from Scratch

### Reason

After incremental additions, the breadboard had become difficult to reason about and remaining display noise could not be cleanly isolated.

### Final Sensitive Subsystem

```text
Tiva C LaunchPad
OLED
DHT11
Soil sensor
LDR + 10 kΩ voltage divider
```

### Final Power Subsystem

```text
LM2596
BC337
1 kΩ base resistor
10 kΩ pull-up resistor
Relay
Pump
1N4007
100 nF
470 µF
```

### Interconnection

```text
PB0 control
single common GND reference
```

### Verification

The rebuilt system operated with stable OLED output and correct pump behavior.

---

## 18. The System Started Irrigation Immediately at Power-Up

### Symptom

For the final demonstration, applying power could start the automatic irrigation logic before the camera was ready.

### Cause

The control algorithm originally became active as soon as the firmware started.

### Solution

LaunchPad SW1 on PF4 was introduced as a manual system enable/disable control.

Final behavior:

```text
Power ON -> sensors/OLED/UART active, pump STOP
SW1 press -> automatic irrigation enabled
Second SW1 press -> pump OFF + state reset + STOP mode
```

### Verification

The final demonstration could be started on command instead of racing the power-up sequence.

---

## 19. Final Verification Strategy

The final system was considered stable only after all of the following were verified together:

- OLED continued updating
- UART continued updating
- DHT11 retained valid data
- soil percentage responded to moisture
- LDR percentage responded to light
- relay switched correctly
- pump turned on only after confirmed dry readings
- pump turned off after confirmed wet readings or safety timeout
- safety timeout was limited to 20 seconds
- SW1 could stop/reset the system
- pump operation no longer produced the severe OLED corruption seen in the earlier layout

---

## Diagnostic Images

Representative evidence is stored under:

```text
images/troubleshooting/
images/uart/
images/oled/
images/soil-sensor/
```

Examples include OLED corruption, temperature errors, pump safety behavior, soil ADC instability, relay/pump wiring, and final UART monitoring.
