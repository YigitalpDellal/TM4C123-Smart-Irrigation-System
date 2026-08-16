# TM4C123 Smart Irrigation System

An embedded smart irrigation system built around the Texas Instruments **EK-TM4C123GXL Tiva C LaunchPad**. The system monitors soil moisture, ambient light, temperature, and humidity, displays live values on an SSD1306 OLED, transmits diagnostic data over UART, and automatically controls a DC water pump through a relay driver.

The final design includes ADC filtering, hysteresis, multi-sample confirmation, a pump safety timeout, manual START/STOP control through the LaunchPad SW1 button, and hardware measures to reduce motor-generated electrical noise.

## Final System

![Final system](images/final-system/01-final-system-hero-view.jpg)

## Main Features

- EK-TM4C123GXL / TM4C123GH6PM based control
- Soil moisture measurement through ADC
- LDR-based ambient light measurement
- DHT11 temperature and humidity sensing
- SSD1306 OLED over I2C0
- UART monitoring at 115200 8N1
- Relay-controlled DC water pump
- BC337 transistor relay driver
- 16-sample ADC averaging
- 35% irrigation start threshold
- 50% irrigation stop threshold
- 3-sample dry confirmation
- 3-sample wet confirmation
- 20-second maximum pump runtime safety limit
- SW1-based manual START/STOP control
- Flyback diode and motor noise suppression
- Physically separated sensitive-electronics and power subsystems

## System Architecture

The final hardware is separated into two functional subsystems.

### Sensitive Electronics Subsystem

- EK-TM4C123GXL LaunchPad
- SSD1306 OLED
- DHT11
- Soil moisture sensor
- LDR + 10 kΩ voltage divider

![Sensitive subsystem](images/sensitive-subsystem/01-sensitive-subsystem-running.jpg)

### Power and Pump Subsystem

- LM2596 buck converter
- BC337 transistor
- 1 kΩ base resistor
- 10 kΩ pull-up resistor
- Relay module
- DC water pump
- 1N4007 flyback diode
- 100 nF ceramic capacitor
- 470 µF electrolytic capacitor

![Power subsystem](images/power-subsystem/10-power-subsystem-complete-view.jpg)

The two subsystems share only the required control/reference connections:

```text
Tiva PB0 --------------------> BC337 base driver path
Tiva GND --------------------> LM2596 OUT- / power-side ground
```

This layout was adopted after motor-generated interference caused OLED corruption and unstable sensor readings in the earlier combined breadboard arrangement.

## Pin Connections

| Function | Tiva C Pin | External Connection |
|---|---|---|
| UART0 RX | PA0 | Stellaris Virtual Serial Port |
| UART0 TX | PA1 | Stellaris Virtual Serial Port |
| DHT11 data | PA2 | DHT11 S |
| Soil ADC | PE3 / AIN0 | Soil sensor AO |
| LDR ADC | PE2 / AIN1 | LDR voltage-divider midpoint |
| Relay control | PB0 | 1 kΩ -> BC337 Base |
| OLED SCL | PB2 / I2C0SCL | SSD1306 SCL |
| OLED SDA | PB3 / I2C0SDA | SSD1306 SDA |
| User switch | PF4 / SW1 | Manual START/STOP |

## LDR Voltage Divider

```text
3.3 V
  |
 LDR
  |
  +--------> PE2 / AIN1
  |
 10 kΩ
  |
 GND
```

The ADC value is converted to a calibrated light percentage after averaging multiple samples.

## Soil Moisture Measurement

The soil sensor analog output is connected to PE3/AIN0.

The calibrated software limits used in the final firmware are:

```c
#define SOIL_DRY_VALUE 4090U
#define SOIL_WET_VALUE 1650U
```

The calculated soil percentage is bounded between 0% and 100%.

## Automatic Irrigation Logic

The irrigation controller does not start automatically at power-up. The system first enters STOP mode while sensor and display monitoring continue.

```text
Power ON
   |
   v
System STOP
   |
SW1 pressed
   |
   v
Automation enabled
   |
   +-- Soil < 35% for 3 consecutive samples --> Pump ON
   |
   +-- Soil > 50% for 3 consecutive samples --> Pump OFF
   |
   +-- Pump runtime reaches 20 s -------------> SAFETY OFF
```

The two separate moisture thresholds create hysteresis and prevent rapid pump switching near a single threshold.

## Pump Driver

The Tiva GPIO does not directly drive the relay input. A BC337 transistor interface is used.

```text
Tiva PB0
   |
  1 kΩ
   |
BC337 Base

LM2596 OUT+ ---- 10 kΩ ----+---- Relay IN
                           |
                     BC337 Collector

BC337 Emitter ------------ LM2596 OUT-
```

Final logical behavior:

```text
PB0 LOW  -> Relay OFF
PB0 HIGH -> Relay ON
```

## Pump Power Path

```text
LM2596 OUT+ -> Relay COM
Relay NO    -> Pump +
Pump -      -> LM2596 OUT-
```

The pump is connected to the relay's normally-open contact so it remains off when the relay is not energized.

## EMI and Noise Mitigation

The DC motor initially caused OLED corruption, DHT11 read failures, ADC spikes, and occasional display freezes. The final design combines hardware and layout improvements:

- 1N4007 flyback diode across the pump power path
- 100 nF ceramic capacitor across the motor terminals
- 470 µF electrolytic capacitor across LM2596 OUT+ / OUT-
- physical separation of sensitive and high-current circuitry
- simplified common-ground connection between subsystems
- filtered ADC measurements
- multi-sample wet/dry confirmation

These changes reduced motor-induced disturbances sufficiently for stable final operation.

## OLED Output

The OLED displays live values such as:

```text
SOIL: 57%
TEMP: 29C
HUM: 32%

LIGHT: 23%

PUMP: OFF
TIME: 0S
```

Before SW1 is pressed, the pump field displays `STOP`.

## UART Output

The same operating state is transmitted through UART for debugging and verification.

Example:

```text
ADC:2695 | Soil:57% | Light:23% | System:RUN | Pump:OFF | Time:0s | Temp:29C | Hum:32%
```

UART remained active throughout development because it was essential for distinguishing display faults, ADC disturbances, DHT11 timing errors, relay behavior, and pump safety logic.

## Safety Features

- Pump is forced OFF at startup
- Automation requires SW1 activation
- Three consecutive dry measurements are required before pump start
- Three consecutive wet measurements are required before normal pump stop
- 20-second maximum pump runtime
- Second SW1 press immediately stops the pump and resets control state
- Pump uses relay NO contact for fail-safe default OFF behavior

## Troubleshooting Highlights

Development exposed several hardware/software integration problems, including:

- incorrect pump/relay contact wiring
- 3.3 V / relay input level incompatibility
- BC337 driver wiring errors
- DHT11 `TEMP: ERR` readings
- OLED display corruption during pump operation
- soil ADC spikes caused by motor interference
- pump restarting because transient soil readings reset the control logic
- duplicated source code causing multiple function definitions and multiple `main()` errors
- ineffective I2C recovery attempts
- complex ground paths in the original breadboard layout

Each problem, diagnosis, implemented correction, and verification step is recorded in [docs/troubleshooting.md](docs/troubleshooting.md).

## Repository Structure

```text
TM4C123-Smart-Irrigation-System/
├── firmware/
│   └── irrigation_system/
├── source/
│   └── main.c
├── docs/
│   ├── report/
│   ├── development-log.md
│   ├── hardware-connections.md
│   ├── project-report.md
│   ├── test-results.md
│   ├── troubleshooting.md
│   └── user-manual.md
├── images/
│   ├── components/
│   ├── dht11/
│   ├── final-system/
│   ├── ldr/
│   ├── oled/
│   ├── power-subsystem/
│   ├── pump/
│   ├── relay-driver/
│   ├── sensitive-subsystem/
│   ├── soil-sensor/
│   ├── troubleshooting/
│   └── uart/
├── videos/
│   ├── subsystem-tests/
│   └── final-demo/
├── .gitignore
├── CHANGELOG.md
├── LICENSE
├── README.md
└── THIRD_PARTY_NOTICES.md
```

## Documentation

- [Hardware connections](docs/hardware-connections.md)
- [Development log](docs/development-log.md)
- [Troubleshooting](docs/troubleshooting.md)
- [Test results](docs/test-results.md)
- [User manual](docs/user-manual.md)

## Development Environment

- Code Composer Studio
- TivaWare for C Series 2.2.0.295
- TI ARM Compiler 20.2.7.LTS
- PuTTY or equivalent serial terminal
- Stellaris ICDI drivers

## UART Configuration

```text
Baud rate:    115200
Data bits:    8
Parity:       None
Stop bits:    1
Flow control: None
```

## Final Status

The final prototype successfully performs live environmental monitoring and automatic irrigation. The OLED and UART remain operational while the pump is controlled according to soil moisture, hysteresis, confirmation counters, manual system enable state, and maximum-runtime safety protection.

## License

The original project source code and documentation are intended to be released under the MIT License. Texas Instruments TivaWare-derived files retain their original license notices and terms; see `THIRD_PARTY_NOTICES.md` when added.