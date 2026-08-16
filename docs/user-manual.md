# User Manual

This document explains how to operate the final TM4C123 Smart Irrigation System prototype.

## 1. Before Power-Up

Verify the following:

- soil probe is connected to the sensor interface
- pump tubing is positioned over the soil
- pump inlet is placed in the water reservoir
- OLED and sensors are connected to the sensitive-electronics breadboard
- relay/pump/LM2596 circuitry is connected to the power breadboard
- Tiva GND and LM2596 OUT- share the intended common reference
- relay COM and NO are used
- 470 µF capacitor polarity is correct
- 1N4007 diode polarity is correct
- pump wiring is secure

Do not change breadboard wiring while the circuit is powered.

## 2. Power-Up

1. Apply power to the pump/power subsystem.
2. Connect/power the EK-TM4C123GXL through the DEBUG USB connection.
3. Open the serial terminal if UART monitoring is required.

The system starts in STOP mode.

Expected OLED state:

```text
SOIL: xx%
TEMP: xxC
HUM: xx%

LIGHT: xx%

PUMP: STOP
TIME: 0S
```

The sensors, OLED, and UART operate while irrigation remains disabled.

## 3. UART Terminal

Use:

```text
Baud rate:    115200
Data bits:    8
Parity:       None
Stop bits:    1
Flow control: None
```

The COM port depends on the computer and can be checked in Windows Device Manager.

## 4. Start Automatic Irrigation

Press LaunchPad **SW1** once.

Expected UART message:

```text
*** SYSTEM STARTED ***
```

The controller now evaluates soil moisture automatically.

## 5. Pump Start Condition

The pump does not start from a single dry sample.

The final condition is:

```text
Soil < 35%
for 3 consecutive control samples
```

After confirmation, the pump switches ON.

OLED example:

```text
PUMP: ON
TIME: 1S
```

## 6. Normal Pump Stop Condition

Normal irrigation ends when:

```text
Soil > 50%
for 3 consecutive samples
```

The pump then turns OFF and runtime returns to zero.

## 7. Safety Shutdown

If the soil does not reach the wet threshold before the maximum allowed runtime, the controller stops the pump at:

```text
20 seconds
```

The display/UART indicates a safety state.

```text
PUMP: SAFETY
TIME: 20S
```

The timeout is a protection mechanism, not the normal irrigation target.

## 8. Stop the System Manually

Press SW1 again while the system is running.

The controller immediately:

- turns the pump OFF
- returns to STOP mode
- resets pump runtime
- resets dry confirmation counter
- resets wet confirmation counter
- clears the safety state

Expected UART message:

```text
*** SYSTEM STOPPED ***
```

## 9. Soil Probe Placement

Insert only the probe section into the soil. Keep the interface electronics outside the soil.

For a representative reading, place the pump outlet a short distance from the probe rather than directing water directly onto the sensor.

This allows the sensor to measure actual soil wetting instead of a direct water stream.

## 10. OLED Fields

```text
SOIL  -> calibrated soil moisture percentage
TEMP  -> DHT11 temperature
HUM   -> DHT11 relative humidity
LIGHT -> calibrated LDR percentage
PUMP  -> STOP / OFF / ON / SAFETY
TIME  -> active pump runtime
```

## 11. If the Pump Does Not Start

Check:

- system has been enabled with SW1
- soil is below 35%
- the dry condition persists for 3 samples
- safety state is not active
- relay LED/mechanical click occurs
- relay COM/NO wiring is correct
- LM2596 output is present
- pump is connected with correct polarity

## 12. If the OLED Does Not Update

Check:

```text
OLED VCC -> 3.3 V
OLED GND -> Tiva GND
OLED SCL -> PB2
OLED SDA -> PB3
```

Also check that power and motor wiring has not been rearranged to recreate the earlier noisy shared-ground layout.

## 13. If DHT11 Shows Invalid Data

DHT11 communication may fail temporarily because it uses timing-sensitive signaling.

The firmware retries the sensor and retains the last valid reading. Persistent errors should trigger checks of:

- PA2 data connection
- 3.3 V supply
- GND
- jumper contact quality

## 14. If Soil Percentage Is Implausible

Check:

```text
Soil AO -> PE3 / AIN0
Soil DO -> not connected
```

Also verify that the probe is making consistent contact with the soil and that high-current pump wiring has not been moved onto the sensitive breadboard.

## 15. Recommended Shutdown

1. Press SW1 to place the system in STOP mode.
2. Confirm pump is OFF.
3. Remove/disable the pump power supply.
4. Disconnect the LaunchPad USB if desired.

This ensures the pump is not active while the hardware is being handled.
