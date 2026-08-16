# TM4C123 Smart Irrigation System - Project Report

## Abstract

This project presents a smart irrigation controller developed with the Texas Instruments EK-TM4C123GXL Tiva C LaunchPad. The system measures soil moisture, ambient light, temperature, and humidity, displays the values on an SSD1306 OLED, transmits diagnostic information over UART, and automatically controls a DC water pump through a relay interface.

The final design combines analog and digital sensing, I2C, UART, GPIO control, timer-based supervision, ADC filtering, hysteresis, multi-sample confirmation, safety shutdown, manual user control, and motor-noise mitigation. A major part of the development involved diagnosing motor-induced interference that affected the OLED, DHT11, and soil ADC measurements. The final hardware was rebuilt as two physically separated subsystems, which significantly improved stability.

## 1. Introduction

Automatic irrigation is a useful embedded-systems application because it requires sensing, decision-making, actuator control, power management, and user feedback in a single closed-loop system.

The project was designed as an extension of an earlier environmental monitoring platform based on the same TM4C123 LaunchPad. The new objective was to turn sensor measurements into a real control action by adding soil-moisture sensing and an electrically isolated pump power path.

## 2. Objectives

The system was required to:

- measure soil moisture
- measure ambient light
- measure temperature and humidity
- show live values locally on an OLED
- transmit live diagnostics over UART
- operate a DC water pump automatically
- avoid rapid pump switching around a threshold
- prevent unlimited pump runtime
- allow manual START/STOP control
- remain stable while the motor is operating

## 3. Hardware

### Main Controller

- EK-TM4C123GXL Tiva C LaunchPad
- TM4C123GH6PM microcontroller

### Sensors and Display

- soil moisture sensor with analog output
- LDR + 10 kΩ voltage divider
- DHT11 temperature/humidity sensor
- SSD1306 I2C OLED

### Actuator and Power Stage

- DC water pump
- relay module
- BC337 NPN transistor
- 1 kΩ base resistor
- 10 kΩ pull-up resistor
- LM2596 buck converter
- 1N4007 diode
- 100 nF ceramic capacitor
- 470 µF electrolytic capacitor

## 4. Pin Assignment

| Function | TM4C123 Pin |
|---|---|
| UART0 RX | PA0 |
| UART0 TX | PA1 |
| DHT11 data | PA2 |
| Relay control | PB0 |
| OLED SCL | PB2 |
| OLED SDA | PB3 |
| LDR analog input | PE2 / AIN1 |
| Soil analog input | PE3 / AIN0 |
| SW1 | PF4 |

## 5. Soil Moisture Measurement

The soil sensor analog output is connected to PE3/AIN0. The ADC value is averaged over multiple samples and converted to a calibrated percentage.

Final calibration constants:

```c
#define SOIL_DRY_VALUE 4090U
#define SOIL_WET_VALUE 1650U
```

The conversion is clamped so readings outside the measured interval remain between 0% and 100%.

## 6. LDR Measurement

The LDR is used in a voltage divider:

```text
3.3 V -> LDR -> PE2/AIN1 -> 10 kΩ -> GND
```

The analog value is averaged and converted into a calibrated light percentage.

## 7. DHT11 Interface

The DHT11 uses a timing-sensitive single-wire digital protocol. The final software includes timeout handling and repeated attempts.

To improve robustness after pump integration:

- the last valid DHT result is retained
- DHT reads are avoided while the pump is actively running

## 8. OLED Interface

The SSD1306 display is connected through I2C0:

```text
PB2 -> SCL
PB3 -> SDA
Address -> 0x3C
```

A compact custom font displays the values required by the interface.

Final fields include:

```text
SOIL
TEMP
HUM
LIGHT
PUMP
TIME
```

## 9. UART Interface

UART0 is configured for:

```text
115200 baud
8 data bits
no parity
1 stop bit
no flow control
```

UART was used both as an output interface and as the primary diagnostic tool throughout development.

## 10. Relay Driver Design

A BC337 interface was used between PB0 and the relay module.

```text
PB0 -> 1 kΩ -> Base
Collector -> Relay IN
Emitter -> LM2596 OUT-
LM2596 OUT+ -> 10 kΩ -> Relay IN
```

This provides a controlled interface between the 3.3 V microcontroller signal and the relay input stage.

## 11. Pump Power Path

The pump is powered through the relay's normally-open contact:

```text
LM2596 OUT+ -> COM
NO -> Pump +
Pump - -> LM2596 OUT-
```

The normally-open contact ensures that the pump is disconnected when the relay is not energized.

## 12. Control Algorithm

The final irrigation algorithm uses both hysteresis and confirmation counters.

### Start Threshold

```text
Soil < 35%
```

The condition must remain true for three consecutive control samples before the pump is enabled.

### Stop Threshold

```text
Soil > 50%
```

The condition must remain true for three consecutive samples before the pump is stopped normally.

### Hysteresis

The difference between 35% and 50% prevents rapid switching around a single threshold.

## 13. ADC Filtering

Pump operation caused transient soil readings during early testing. The final implementation averages 16 ADC samples before calculating percentages.

This was combined with consecutive-sample confirmation because averaging alone could not guarantee that one disturbed measurement would never affect the control decision.

## 14. Pump Safety Timeout

The pump has a maximum continuous runtime of 20 seconds.

```c
#define MAX_PUMP_RUNTIME 20U
```

The safety timeout is not the normal irrigation stop mechanism. It is only used if the soil does not reach the wet threshold in time.

## 15. SW1 Manual Control

The system originally enabled automatic irrigation immediately after startup. This made controlled demonstrations difficult and reduced operator control.

SW1 on PF4 was added as a manual enable/reset interface.

```text
Power ON -> STOP mode
First press -> RUN
Second press -> STOP and reset
```

While stopped, sensor monitoring, OLED output, and UART remain active.

## 16. Motor Noise and EMI Problems

The pump integration produced the most significant technical challenge.

Observed problems included:

- OLED text corruption
- OLED freezing on an old value
- DHT11 errors
- soil ADC spikes
- unstable pump state transitions

These symptoms appeared mainly when the motor started or ran.

## 17. Hardware Noise Mitigation

The final power path includes:

### Flyback Diode

A 1N4007 is connected across the pump path in reverse bias during normal operation.

### Motor Capacitor

A 100 nF ceramic capacitor is connected across the pump terminals to reduce high-frequency motor noise.

### Bulk Capacitor

A 470 µF electrolytic capacitor is connected across LM2596 OUT+ and OUT- to reduce supply disturbances.

## 18. Grounding and Physical Layout

As the original prototype evolved, multiple ground and power paths accumulated on the breadboard. Removing some redundant ground paths improved the OLED, suggesting that the physical return-current layout was contributing to the problem.

The final system was therefore rebuilt as two separate subsystems.

### Sensitive Subsystem

```text
Tiva C
OLED
DHT11
Soil sensor
LDR divider
```

### Power Subsystem

```text
LM2596
BC337
Relay
Pump
1N4007
100 nF
470 µF
```

The two sides share the required PB0 control signal and one common reference connection.

## 19. Troubleshooting Process

The project did not reach the final configuration in one step. Important development failures included:

- relay not switching correctly from the original control arrangement
- BC337 wiring mistakes
- incorrect early pump contact wiring
- pump stopping/restarting because of noisy ADC readings
- DHT11 error states
- OLED corruption during pump operation
- OLED freeze while UART continued running
- ineffective software-only I2C recovery
- duplicated source code producing repeated-function and multiple-main errors
- overly short initial safety timeout
- complex ground paths

The complete troubleshooting history is documented in `docs/troubleshooting.md`.

## 20. Test Results

The final prototype successfully demonstrated:

- stable soil measurement
- stable light measurement
- valid temperature and humidity monitoring
- live OLED display
- continuous UART output
- relay control through BC337
- pump control through relay NO contact
- filtered ADC behavior
- hysteresis
- dry/wet confirmation counters
- safety shutdown at 20 seconds
- SW1-controlled START/STOP
- stable operation after final hardware rebuild

Detailed results are available in `docs/test-results.md`.

## 21. Limitations

- DHT11 accuracy and update speed are limited.
- Soil-sensor calibration depends on the specific probe and soil conditions.
- LDR calibration depends on the physical setup and lighting environment.
- The prototype is constructed on breadboards rather than a PCB.
- The resistive soil sensor may corrode during long-term use.
- The firmware uses several blocking operations.
- No historical data logging is implemented.
- No wireless connectivity is included.

## 22. Future Improvements

Potential extensions include:

- capacitive soil moisture sensor
- MOSFET pump driver instead of a mechanical relay
- dedicated motor supply filtering
- PCB implementation with controlled grounding
- non-blocking DHT driver
- RTOS or event-driven architecture
- data logging
- Wi-Fi/Bluetooth connectivity
- configurable moisture thresholds
- OLED menu system
- water-level sensing
- RTC-based irrigation schedules
- current sensing for pump fault detection

## 23. Conclusion

The final system demonstrates a complete embedded closed-loop application on the TM4C123 platform. It integrates analog sensors, a timing-sensitive digital sensor, I2C display communication, UART debugging, GPIO control, relay/pump actuation, safety logic, filtering, and manual control.

The most important engineering result was not merely that the pump could be switched. The project demonstrated how actuator integration can introduce electrical and software-level failures into otherwise working sensor electronics, and how those failures can be isolated through UART diagnostics, filtering, hardware suppression, grounding changes, and physical subsystem separation.

The final rebuilt prototype operates as a stable smart irrigation demonstrator and provides a reproducible record of the development and troubleshooting process.