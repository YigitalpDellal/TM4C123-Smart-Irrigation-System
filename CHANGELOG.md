# Changelog

## v1.0.0 - Final Prototype

### Added

- Soil moisture sensing on PE3/AIN0
- LDR voltage-divider measurement on PE2/AIN1
- DHT11 temperature and humidity sensing on PA2
- SSD1306 OLED output over I2C0
- UART monitoring over UART0 at 115200 8N1
- BC337-based relay driver controlled from PB0
- Relay-switched DC water pump
- 16-sample ADC averaging
- Soil-moisture calibration
- 35% irrigation start threshold
- 50% irrigation stop threshold
- 3-sample dry confirmation
- 3-sample wet confirmation
- 20-second pump safety timeout
- SW1/PF4 START/STOP control
- Last-valid-value retention for DHT11 data
- Pump-time DHT read suppression
- 1N4007 flyback protection
- 100 nF motor noise-suppression capacitor
- 470 µF LM2596 output capacitor
- Two-breadboard sensitive/power subsystem separation
- Comprehensive troubleshooting documentation
- Hardware connection guide
- Test-results documentation
- User manual

### Changed

- Replaced immediate power-up automation with SW1-controlled enable state
- Reworked relay interface from direct control attempts to BC337 level/interface stage
- Increased pump safety timeout from early 10-second testing value to final 20 seconds
- Reworked control logic so soil moisture is the normal stop criterion and timeout is only a safety mechanism
- Rebuilt hardware to simplify current-return paths and isolate motor/power circuitry from sensitive sensor/display circuitry

### Fixed

- Incorrect early relay COM/NO pump wiring
- Unstable pump restart/stop behavior caused by transient soil readings
- OLED corruption during pump operation
- OLED freeze while UART remained active
- DHT11 transient error handling
- Excessive sensitivity to single ADC spikes
- Duplicate source-code definitions and multiple `main()` build errors
- Ineffective software-only OLED/I2C recovery attempt

## Development History

Earlier intermediate versions were experimental and are described in `docs/development-log.md` and `docs/troubleshooting.md` rather than published as separate stable releases.