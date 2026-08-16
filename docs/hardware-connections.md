# Hardware Connections

This document describes the final verified wiring of the TM4C123 Smart Irrigation System.

## 1. EK-TM4C123GXL LaunchPad

The LaunchPad is the main controller. It handles ADC sampling, DHT11 timing, OLED communication, UART output, relay control, and SW1 user input.

## 2. OLED Display

```text
OLED GND -> Tiva GND
OLED VCC -> Tiva 3.3 V
OLED SCL -> PB2 / I2C0SCL
OLED SDA -> PB3 / I2C0SDA
```

A 100 nF local decoupling capacitor may be placed between OLED VCC and GND close to the display if required.

## 3. DHT11

```text
DHT11 S -> PA2
DHT11 + -> Tiva 3.3 V
DHT11 - -> Tiva GND
```

## 4. Soil Moisture Sensor

```text
Soil + / VCC -> Tiva 3.3 V
Soil - / GND -> Tiva GND
Soil AO      -> PE3 / AIN0
Soil DO      -> not connected
```

Only the analog output is used.

## 5. LDR Voltage Divider

```text
Tiva 3.3 V
   |
  LDR
   |
   +--------> PE2 / AIN1
   |
 10 kΩ
   |
  GND
```

The LDR, PE2 jumper, and one side of the 10 kΩ resistor share the same midpoint node.

## 6. BC337 Relay Driver

With the verified transistor orientation used in the prototype, the driver is wired as:

```text
PB0 -> 1 kΩ -> BC337 Base
BC337 Emitter -> LM2596 OUT-
BC337 Collector -> Relay IN
LM2596 OUT+ -> 10 kΩ -> Relay IN / Collector node
```

The 1 kΩ resistor limits base current. The 10 kΩ resistor provides the relay-input pull-up.

## 7. Relay Module Supply

```text
Relay VCC -> LM2596 OUT+
Relay GND -> LM2596 OUT-
Relay IN  -> BC337 Collector node
```

## 8. Pump Power Path

```text
LM2596 OUT+ -> Relay COM
Relay NO    -> Pump red / +
Pump black / - -> LM2596 OUT-
```

The normally-open contact is used so the pump is OFF when the relay is inactive.

## 9. Flyback Diode

The 1N4007 is connected across the pump supply path in reverse bias during normal operation:

```text
1N4007 striped end   -> Pump + / Relay NO side
1N4007 non-striped end -> Pump - / LM2596 OUT-
```

The stripe marks the cathode.

## 10. Motor Noise-Suppression Capacitor

A 100 nF ceramic capacitor is placed across the pump terminals:

```text
Pump + ---- 100 nF ---- Pump -
```

Ceramic capacitors are non-polarized.

## 11. LM2596 Bulk Capacitor

A 470 µF electrolytic capacitor is connected across the LM2596 output:

```text
470 µF + -> LM2596 OUT+
470 µF - -> LM2596 OUT-
```

The negative stripe on the capacitor body identifies the negative terminal.

## 12. Common Ground Between Subsystems

The sensitive electronics and power subsystem require a shared electrical reference for the PB0 control signal.

Final connection:

```text
Tiva GND -> LM2596 OUT-
```

The final layout intentionally avoids multiple unnecessary ground bridges between the two breadboards.

## 13. SW1 User Button

LaunchPad SW1 is connected internally to PF4 and is configured with the internal pull-up resistor.

```text
Released -> HIGH
Pressed  -> LOW
```

Software behavior:

```text
First press  -> START irrigation automation
Second press -> STOP, pump OFF, reset counters and safety state
```

## 14. Final Pin Summary

| Tiva Pin | Function |
|---|---|
| PA0 | UART0 RX |
| PA1 | UART0 TX |
| PA2 | DHT11 data |
| PB0 | Relay driver control |
| PB2 | OLED I2C SCL |
| PB3 | OLED I2C SDA |
| PE2 | LDR ADC / AIN1 |
| PE3 | Soil ADC / AIN0 |
| PF4 | SW1 user input |

## 15. Power-Up Check

Before applying power, verify:

- no direct short between OUT+ and OUT-
- electrolytic capacitor polarity is correct
- diode stripe is on the pump-positive side
- relay COM/NO are used, not NC
- pump polarity is correct
- PB0 reaches the BC337 base only through the 1 kΩ resistor
- OUT+ reaches Relay IN only through the 10 kΩ resistor
- Tiva and LM2596 share one common ground reference
- OLED and sensors remain on the sensitive 3.3 V side
