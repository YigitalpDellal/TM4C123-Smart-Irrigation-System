# TM4C123 Smart Irrigation System

An embedded closed-loop irrigation controller built around the Texas Instruments **EK-TM4C123GXL Tiva C LaunchPad**. The system measures soil moisture, ambient light, temperature, and humidity, displays live status on an SSD1306 OLED, streams diagnostics over UART, and automatically controls a DC water pump through a BC337-driven relay interface.

The final prototype combines sensor calibration, 16-sample ADC averaging, hysteresis, consecutive-sample confirmation, a 20-second pump safety timeout, SW1 manual START/STOP control, and hardware changes introduced to suppress motor-generated interference.

<p align="center">
<img src="images/final-system/01-final-system-hero-view.jpg" width="92%" alt="Final TM4C123 smart irrigation system">
</p>

## Project Links

- **Final readable source:** [`source/main.c`](source/main.c)
- **Importable CCS project:** [`firmware/irrigation_system/`](firmware/irrigation_system/)
- **Project report:** [`docs/project-report.md`](docs/project-report.md)
- **Hardware connections:** [`docs/hardware-connections.md`](docs/hardware-connections.md)
- **Troubleshooting record:** [`docs/troubleshooting.md`](docs/troubleshooting.md)
- **Troubleshooting coverage audit:** [`docs/troubleshooting-audit.md`](docs/troubleshooting-audit.md)
- **Test results:** [`docs/test-results.md`](docs/test-results.md)
- **Build and run guide:** [`docs/build-and-run.md`](docs/build-and-run.md)
- **Evidence guide:** [`docs/media-evidence.md`](docs/media-evidence.md)
- **Complete media inventory:** [`docs/media-inventory.md`](docs/media-inventory.md)

## Project at a Glance

| Item | Final implementation |
|---|---|
| Controller | EK-TM4C123GXL / TM4C123GH6PM |
| Soil sensing | PE3 / AIN0, calibrated 0–100% |
| Light sensing | PE2 / AIN1, LDR divider |
| Temperature / humidity | DHT11 on PA2 |
| Local display | SSD1306 OLED on I2C0 |
| Diagnostics | UART0, 115200 8N1 |
| Pump control | PB0 → 1 kΩ → BC337 → relay input |
| Irrigation start | Soil < 35% for 3 consecutive samples |
| Normal stop | Soil > 50% for 3 consecutive samples |
| ADC filtering | 16-sample averaging |
| Safety limit | 20 s maximum continuous pump runtime |
| Manual control | SW1 / PF4 START–STOP toggle |

## Video Demonstrations

The repository contains seven optimized H.264/MP4 validation recordings. Click a preview image to open the corresponding video.

<table>
<tr>
<td align="center" width="50%"><a href="https://raw.githubusercontent.com/YigitalpDellal/TM4C123-Smart-Irrigation-System/main/videos/final-demo/01-final-irrigation-demo.mp4"><img src="images/final-system/01-final-system-hero-view.jpg" width="100%"></a><br><b>Final Irrigation Demonstration</b></td>
<td align="center" width="50%"><a href="https://raw.githubusercontent.com/YigitalpDellal/TM4C123-Smart-Irrigation-System/main/videos/subsystem-tests/01-soil-moisture-water-test.mp4"><img src="images/soil-sensor/09-soil-sensor-and-irrigation-outlet-frame.jpg" width="100%"></a><br><b>Soil-Moisture / Water Test</b></td>
</tr>
<tr>
<td align="center" width="50%"><a href="https://raw.githubusercontent.com/YigitalpDellal/TM4C123-Smart-Irrigation-System/main/videos/subsystem-tests/02-oled-sensor-subsystem-test.mp4"><img src="images/sensitive-subsystem/05-sensitive-subsystem-live-display-frame.jpg" width="100%"></a><br><b>OLED and Sensor Subsystem Test</b></td>
<td align="center" width="50%"><a href="https://raw.githubusercontent.com/YigitalpDellal/TM4C123-Smart-Irrigation-System/main/videos/subsystem-tests/03-relay-driver-switching-test.mp4"><img src="images/relay-driver/07-relay-switching-test-frame.jpg" width="100%"></a><br><b>Relay-Driver Switching Test</b></td>
</tr>
<tr>
<td align="center" width="50%"><a href="https://raw.githubusercontent.com/YigitalpDellal/TM4C123-Smart-Irrigation-System/main/videos/subsystem-tests/04-pump-display-integration-test.mp4"><img src="images/pump/07-pump-and-oled-integration-test-frame.jpg" width="100%"></a><br><b>Pump and Display Integration Test</b></td>
<td align="center" width="50%"><a href="https://raw.githubusercontent.com/YigitalpDellal/TM4C123-Smart-Irrigation-System/main/videos/subsystem-tests/05-sensitive-subsystem-soil-test.mp4"><img src="images/sensitive-subsystem/01-sensitive-subsystem-running.jpg" width="100%"></a><br><b>Sensitive-Subsystem Soil Test</b></td>
</tr>
<tr>
<td align="center" width="50%"><a href="https://raw.githubusercontent.com/YigitalpDellal/TM4C123-Smart-Irrigation-System/main/videos/subsystem-tests/06-integrated-system-control-test.mp4"><img src="images/final-system/10-sw1-start-stop-control-frame.jpg" width="100%"></a><br><b>Integrated System Control Test</b></td>
<td align="center" width="50%"><a href="videos/final-demo/01-final-irrigation-demo.mp4"><img src="images/final-system/09-complete-irrigation-setup-with-reservoir.jpg" width="100%"></a><br><b>Final Demo Repository File</b></td>
</tr>
</table>

## Main Features

- EK-TM4C123GXL / TM4C123GH6PM based controller
- Soil moisture measurement through ADC0 / AIN0
- LDR-based ambient-light measurement through ADC0 / AIN1
- DHT11 temperature and humidity sensing
- SSD1306 OLED over I2C0
- UART0 monitoring at 115200 8N1
- BC337-based relay input driver
- Relay-controlled DC water pump
- 16-sample ADC averaging
- 35% / 50% hysteresis band
- 3-sample dry and wet confirmation
- 20-second maximum pump runtime safety lock
- SW1-based manual START/STOP control
- Flyback/noise-suppression hardware and bulk decoupling
- Physically separated sensitive-electronics and power subsystems

## System Architecture

<table>
<tr>
<td width="50%" valign="top"><b>Sensitive Electronics</b><br>LaunchPad, OLED, DHT11, soil sensor, LDR divider<br><br><img src="images/sensitive-subsystem/01-sensitive-subsystem-running.jpg" width="100%"></td>
<td width="50%" valign="top"><b>Power / Pump Subsystem</b><br>LM2596, BC337, relay, pump, diode and filtering<br><br><img src="images/power-subsystem/10-power-subsystem-complete-view.jpg" width="100%"></td>
</tr>
</table>

```text
Tiva PB0 --------------------> BC337 base-driver path
Tiva GND --------------------> LM2596 OUT- / power-side ground
```

The two-subsystem architecture was adopted after pump-related electrical interference produced OLED corruption, DHT11 failures and unstable ADC readings in the earlier combined breadboard arrangement.

## Pin Connections

| Function | Tiva C Pin | External Connection |
|---|---|---|
| UART0 RX | PA0 | Stellaris Virtual Serial Port |
| UART0 TX | PA1 | Stellaris Virtual Serial Port |
| DHT11 data | PA2 | DHT11 S |
| Relay control | PB0 | 1 kΩ → BC337 Base |
| OLED SCL | PB2 / I2C0SCL | SSD1306 SCL |
| OLED SDA | PB3 / I2C0SDA | SSD1306 SDA |
| LDR ADC | PE2 / AIN1 | LDR divider midpoint |
| Soil ADC | PE3 / AIN0 | Soil sensor AO |
| User switch | PF4 / SW1 | Manual START/STOP |

## Sensor Calibration

```c
#define SOIL_DRY_VALUE 4090U
#define SOIL_WET_VALUE 1650U
```

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

## Automatic Irrigation Logic

```text
Power ON
   |
   v
System STOP / Pump OFF
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

The 35% / 50% thresholds create hysteresis. Consecutive-sample confirmation prevents a single motor-induced ADC spike from immediately changing the pump state.

## Relay Driver and Pump Power Path

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

```text
LM2596 OUT+ -> Relay COM
Relay NO    -> Pump +
Pump -      -> LM2596 OUT-
```

## EMI and Noise Mitigation

Motor integration produced the hardest failures in the project: OLED corruption, OLED freezes, DHT11 errors, soil ADC spikes and unstable pump state transitions. The final solution combines hardware and software mitigation:

- 1N4007 diode across the pump path
- 100 nF ceramic capacitor across the motor terminals
- 470 µF electrolytic capacitor across LM2596 OUT+ / OUT-
- separated sensitive and high-current breadboards
- deliberate common-ground/reference connection
- 16-sample ADC averaging
- three-sample wet/dry confirmation
- DHT11 reads deferred while the pump is running
- last valid DHT11 value retained after transient read failures

<p align="center">
<img src="images/power-subsystem/13-1n4007-flyback-diode-close-up.jpg" width="24%">
<img src="images/power-subsystem/14-100nf-ceramic-capacitor-close-up.jpg" width="24%">
<img src="images/power-subsystem/12-470uf-capacitor-close-up.jpg" width="24%">
<img src="images/power-subsystem/17-common-ground-connection-close-up.jpg" width="24%">
</p>

## Closed-Loop Verification

### Dry condition → Pump ON
![Dry soil trigger](images/uart/04-dry-soil-trigger-pump-on.png)

### Wet condition → Normal Pump OFF
![Wet soil normal stop](images/uart/05-wet-soil-normal-pump-off.png)

### Safety timeout
![Safety state](images/uart/03-final-uart-monitoring-safety-state.png)

## Safety Features

- Pump output forced OFF at startup
- Automation requires an SW1 press
- Three consecutive dry measurements required before pump start
- Three consecutive wet measurements required before normal pump stop
- 20-second maximum continuous pump runtime
- Safety state prevents immediate restart after timeout
- Second SW1 press immediately disables the pump and resets controller state
- Relay NO contact provides a default-open pump path

## Engineering Problems Solved

The project documentation retains the failures as well as the final result: soil-calibration issues, threshold chatter, relay-interface problems, BC337 wiring mistakes, incorrect relay contact wiring, timeout refinement, motor-induced ADC noise, OLED corruption and freezing, DHT11 errors, unsuccessful software-only I2C recovery, custom-font limitations, duplicate-source build errors, compiler error cascades, grounding problems, the two-breadboard rebuild, SW1-controlled startup and final integrated verification.

See [`docs/troubleshooting.md`](docs/troubleshooting.md) and [`docs/troubleshooting-audit.md`](docs/troubleshooting-audit.md) for the full symptom → cause → correction → verification record.

## Complete Visual Documentation

Every curated repository image is shown below. This README therefore acts as both a technical overview and the complete visual build record. **All 99 project images are included.**

### Final System
<p align="center">
<img src="images/final-system/01-final-system-hero-view.jpg" width="32%">
<img src="images/final-system/02-final-system-running-angle.jpg" width="32%">
<img src="images/final-system/03-final-irrigation-system-wide-view.jpg" width="32%">
<img src="images/final-system/04-final-system-top-view.jpg" width="32%">
<img src="images/final-system/05-final-system-vertical-overview.jpg" width="32%">
<img src="images/final-system/06-final-system-full-layout.jpg" width="32%">
<img src="images/final-system/07-final-system-top-view-additional.jpg" width="32%">
<img src="images/final-system/08-integrated-hardware-overview-additional.jpg" width="32%">
<img src="images/final-system/09-complete-irrigation-setup-with-reservoir.jpg" width="32%">
<img src="images/final-system/10-sw1-start-stop-control-frame.jpg" width="32%">
</p>

### Component Reference
<p align="center">
<img src="images/components/01-ek-tm4c123gxl-launchpad.jpg" width="32%">
<img src="images/components/02-ssd1306-oled-display.jpg" width="32%">
<img src="images/components/03-dht11-sensor.jpg" width="32%">
<img src="images/components/04-soil-moisture-interface-module.jpg" width="32%">
<img src="images/components/05-soil-moisture-probe.jpg" width="32%">
<img src="images/components/06-ldr-photoresistor.jpg" width="32%">
<img src="images/components/07-relay-module.jpg" width="32%">
<img src="images/components/08-bc337-transistor.jpg" width="32%">
<img src="images/components/09-lm2596-buck-converter.jpg" width="32%">
<img src="images/components/10-dc-water-pump.jpg" width="32%">
<img src="images/components/11-1n4007-diode.jpg" width="32%">
<img src="images/components/12-resistor-a.jpg" width="32%">
<img src="images/components/13-resistor-b.jpg" width="32%">
<img src="images/components/14-100nf-ceramic-capacitor.jpg" width="32%">
<img src="images/components/15-470uf-electrolytic-capacitor.jpg" width="32%">
<img src="images/components/16-battery-holder.jpg" width="32%">
</p>

### Sensitive Electronics Subsystem
<p align="center">
<img src="images/sensitive-subsystem/01-sensitive-subsystem-running.jpg" width="32%">
<img src="images/sensitive-subsystem/02-sensitive-subsystem-top-view.jpg" width="32%">
<img src="images/sensitive-subsystem/03-sensitive-subsystem-angle-view.jpg" width="32%">
<img src="images/sensitive-subsystem/04-sensitive-subsystem-integration.jpg" width="32%">
<img src="images/sensitive-subsystem/05-sensitive-subsystem-live-display-frame.jpg" width="32%">
</p>

### Power and Pump Subsystem
<p align="center">
<img src="images/power-subsystem/01-power-subsystem-overview.jpg" width="32%">
<img src="images/power-subsystem/02-relay-and-power-wiring.jpg" width="32%">
<img src="images/power-subsystem/03-pump-protection-wiring-close-up.jpg" width="32%">
<img src="images/power-subsystem/04-relay-lm2596-integration.jpg" width="32%">
<img src="images/power-subsystem/05-pump-noise-suppression-network.jpg" width="32%">
<img src="images/power-subsystem/06-lm2596-output-filter-wiring.jpg" width="32%">
<img src="images/power-subsystem/07-flyback-diode-close-up.jpg" width="32%">
<img src="images/power-subsystem/08-power-path-integrated-view.jpg" width="32%">
<img src="images/power-subsystem/09-driver-resistor-wiring-close-up.jpg" width="32%">
<img src="images/power-subsystem/10-power-subsystem-complete-view.jpg" width="32%">
<img src="images/power-subsystem/11-bulk-capacitor-integration.jpg" width="32%">
<img src="images/power-subsystem/12-470uf-capacitor-close-up.jpg" width="32%">
<img src="images/power-subsystem/13-1n4007-flyback-diode-close-up.jpg" width="32%">
<img src="images/power-subsystem/14-100nf-ceramic-capacitor-close-up.jpg" width="32%">
<img src="images/power-subsystem/15-relay-lm2596-power-subsystem-view.jpg" width="32%">
<img src="images/power-subsystem/16-power-subsystem-alternate-view.jpg" width="32%">
<img src="images/power-subsystem/17-common-ground-connection-close-up.jpg" width="32%">
<img src="images/power-subsystem/18-power-subsystem-running-frame.jpg" width="32%">
</p>

### Soil Sensor and Irrigation
<p align="center">
<img src="images/soil-sensor/01-soil-sensor-interface-wiring.jpg" width="32%">
<img src="images/soil-sensor/02-soil-probe-installed-in-soil.jpg" width="32%">
<img src="images/soil-sensor/03-soil-sensor-tiva-connection.jpg" width="32%">
<img src="images/soil-sensor/04-soil-sensor-breadboard-overview.jpg" width="32%">
<img src="images/soil-sensor/05-soil-dry-uart-test.png" width="32%">
<img src="images/soil-sensor/06-soil-wet-dry-transition-uart.png" width="32%">
<img src="images/soil-sensor/07-soil-intermediate-moisture-uart.png" width="32%">
<img src="images/soil-sensor/08-soil-safety-off-uart.png" width="32%">
<img src="images/soil-sensor/09-soil-sensor-and-irrigation-outlet-frame.jpg" width="32%">
</p>

### LDR / Ambient Light
<p align="center">
<img src="images/ldr/01-ldr-voltage-divider-overview.jpg" width="32%">
<img src="images/ldr/02-ldr-voltage-divider-close-up.jpg" width="32%">
<img src="images/ldr/03-ldr-uart-test-condition-a.png" width="32%">
<img src="images/ldr/04-ldr-uart-test-condition-b.png" width="32%">
</p>

### DHT11 Temperature and Humidity
<p align="center">
<img src="images/dht11/01-dht11-and-oled-running.jpg" width="32%">
<img src="images/dht11/02-dht11-breadboard-wiring-top-view.jpg" width="32%">
<img src="images/dht11/03-dht11-breadboard-wiring-angle.jpg" width="32%">
<img src="images/dht11/04-dht11-uart-readings.png" width="32%">
</p>

### OLED and Display Evidence
<p align="center">
<img src="images/oled/01-oled-live-display-close-up.jpg" width="32%">
<img src="images/oled/02-oled-i2c-wiring-close-up.jpg" width="32%">
<img src="images/oled/03-oled-breadboard-integration.jpg" width="32%">
<img src="images/oled/04-oled-early-integration-view.jpg" width="32%">
<img src="images/oled/05-oled-temperature-error-uart.png" width="32%">
<img src="images/oled/06-oled-uart-operating-output.png" width="32%">
<img src="images/oled/07-final-oled-stop-state-close-up.jpg" width="32%">
</p>

### BC337 Relay Driver
<p align="center">
<img src="images/relay-driver/01-bc337-driver-early-test.jpg" width="32%">
<img src="images/relay-driver/02-relay-driver-system-test.jpg" width="32%">
<img src="images/relay-driver/03-bc337-resistor-network-close-up.jpg" width="32%">
<img src="images/relay-driver/04-relay-and-lm2596-wiring.jpg" width="32%">
<img src="images/relay-driver/05-bc337-driver-breadboard-view.jpg" width="32%">
<img src="images/relay-driver/06-relay-driver-integrated-view.jpg" width="32%">
<img src="images/relay-driver/07-relay-switching-test-frame.jpg" width="32%">
</p>

### Pump and Water Path
<p align="center">
<img src="images/pump/01-irrigation-outlet-at-soil.jpg" width="32%">
<img src="images/pump/02-pump-noise-suppression-network.jpg" width="32%">
<img src="images/pump/03-pump-wiring-close-up.jpg" width="32%">
<img src="images/pump/04-flyback-diode-across-pump-line.jpg" width="32%">
<img src="images/pump/05-pump-power-supply-connection.jpg" width="32%">
<img src="images/pump/06-pump-relay-power-path.jpg" width="32%">
<img src="images/pump/07-pump-and-oled-integration-test-frame.jpg" width="32%">
</p>

### Troubleshooting Evidence
<p align="center">
<img src="images/troubleshooting/01-early-relay-driver-test.jpg" width="32%">
<img src="images/troubleshooting/02-pump-safety-off-behavior.png" width="32%">
<img src="images/troubleshooting/03-temperature-error-and-pump-cycles.png" width="32%">
<img src="images/troubleshooting/04-initial-pump-relay-wiring.png" width="32%">
<img src="images/troubleshooting/05-soil-adc-instability-during-pump-operation.png" width="32%">
<img src="images/troubleshooting/06-oled-display-corruption-close-up.png" width="32%">
<img src="images/troubleshooting/07-oled-display-corruption-after-pump-operation.png" width="32%">
</p>

### Final UART Evidence
<p align="center">
<img src="images/uart/01-final-uart-monitoring-a.png" width="32%">
<img src="images/uart/02-final-uart-monitoring-b.png" width="32%">
<img src="images/uart/03-final-uart-monitoring-safety-state.png" width="32%">
<img src="images/uart/04-dry-soil-trigger-pump-on.png" width="32%">
<img src="images/uart/05-wet-soil-normal-pump-off.png" width="32%">
</p>

## Repository Structure

```text
TM4C123-Smart-Irrigation-System/
├── firmware/
│   └── irrigation_system/        # Importable Code Composer Studio project
├── source/
│   └── main.c                    # Final readable application source
├── docs/
│   ├── build-and-run.md
│   ├── development-log.md
│   ├── hardware-connections.md
│   ├── media-evidence.md
│   ├── media-inventory.md
│   ├── project-report.md
│   ├── test-results.md
│   ├── troubleshooting-audit.md
│   ├── troubleshooting.md
│   └── user-manual.md
├── images/                       # 99 curated project images
├── videos/
│   ├── subsystem-tests/
│   └── final-demo/
├── .gitattributes
├── .gitignore
├── CHANGELOG.md
├── LICENSE
├── README.md
└── THIRD_PARTY_NOTICES.md
```

## Development Environment

- Code Composer Studio
- TivaWare for C Series 2.2.0.295
- TI ARM Compiler 20.2.7.LTS
- Stellaris ICDI debug interface
- PuTTY or another serial terminal

```text
Baud rate:    115200
Data bits:    8
Parity:       None
Stop bits:    1
Flow control: None
```

See [`docs/build-and-run.md`](docs/build-and-run.md) for import, build, flash and serial-monitor instructions.

## Final Status

The completed prototype demonstrates a full embedded closed loop: sensing → filtering → calibrated interpretation → decision logic → relay/pump actuation → physical soil wetting → new sensor measurement. OLED and UART remain available for observation while the pump is controlled according to calibrated soil moisture, hysteresis, confirmation counters, operator enable state and maximum-runtime safety protection.

The readable source and the CCS project contain the same final application code.

## License

Original project source code and documentation are released under the MIT License. Texas Instruments-provided or TivaWare-derived files retain their original license notices and terms. See [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) for details.
