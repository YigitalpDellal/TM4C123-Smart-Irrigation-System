# Media Evidence Index

This document maps the repository photographs, UART screenshots, and videos to the engineering claims they support.

## 1. Final System Overview

- [`images/final-system/01-final-system-hero-view.jpg`](../images/final-system/01-final-system-hero-view.jpg) — main final-system overview
- [`images/final-system/04-final-system-top-view.jpg`](../images/final-system/04-final-system-top-view.jpg) — top-view system layout
- [`images/final-system/09-complete-irrigation-setup-with-reservoir.jpg`](../images/final-system/09-complete-irrigation-setup-with-reservoir.jpg) — complete irrigation setup including reservoir/pump context
- [`images/final-system/10-sw1-start-stop-control-frame.jpg`](../images/final-system/10-sw1-start-stop-control-frame.jpg) — frame captured from validation video showing SW1/manual-control context

## 2. Sensitive Electronics Subsystem

- [`images/sensitive-subsystem/01-sensitive-subsystem-running.jpg`](../images/sensitive-subsystem/01-sensitive-subsystem-running.jpg) — sensitive subsystem operating
- [`images/sensitive-subsystem/02-sensitive-subsystem-top-view.jpg`](../images/sensitive-subsystem/02-sensitive-subsystem-top-view.jpg) — subsystem layout
- [`images/sensitive-subsystem/05-sensitive-subsystem-live-display-frame.jpg`](../images/sensitive-subsystem/05-sensitive-subsystem-live-display-frame.jpg) — validation-video frame with live display

## 3. OLED Verification

- [`images/oled/01-oled-live-display-close-up.jpg`](../images/oled/01-oled-live-display-close-up.jpg) — OLED operating close-up
- [`images/oled/07-final-oled-stop-state-close-up.jpg`](../images/oled/07-final-oled-stop-state-close-up.jpg) — final STOP-state display with live sensor fields
- [`images/oled/05-oled-temperature-error-uart.png`](../images/oled/05-oled-temperature-error-uart.png) — DHT/OLED error evidence from development

## 4. Soil Sensor and Irrigation Logic

- [`images/soil-sensor/01-soil-sensor-interface-wiring.jpg`](../images/soil-sensor/01-soil-sensor-interface-wiring.jpg) — analog sensor interface wiring
- [`images/soil-sensor/02-soil-probe-installed-in-soil.jpg`](../images/soil-sensor/02-soil-probe-installed-in-soil.jpg) — probe installed in soil
- [`images/soil-sensor/09-soil-sensor-and-irrigation-outlet-frame.jpg`](../images/soil-sensor/09-soil-sensor-and-irrigation-outlet-frame.jpg) — video-derived frame showing probe and irrigation outlet together
- [`images/soil-sensor/05-soil-dry-uart-test.png`](../images/soil-sensor/05-soil-dry-uart-test.png) — dry-condition UART evidence
- [`images/soil-sensor/06-soil-wet-dry-transition-uart.png`](../images/soil-sensor/06-soil-wet-dry-transition-uart.png) — wet/dry transition evidence
- [`images/soil-sensor/08-soil-safety-off-uart.png`](../images/soil-sensor/08-soil-safety-off-uart.png) — pump safety-timeout evidence

## 5. Normal Closed-Loop Pump Verification

- [`images/uart/04-dry-soil-trigger-pump-on.png`](../images/uart/04-dry-soil-trigger-pump-on.png) — dry soil causes pump ON after the controller accepts the dry state
- [`images/uart/05-wet-soil-normal-pump-off.png`](../images/uart/05-wet-soil-normal-pump-off.png) — moisture rises and the pump returns to OFF through the normal wet-stop path
- [`images/uart/03-final-uart-monitoring-safety-state.png`](../images/uart/03-final-uart-monitoring-safety-state.png) — separate safety-state evidence

Together, the first two UART screenshots are the primary evidence for the intended normal closed-loop irrigation sequence rather than the emergency timeout path.

## 6. LDR Measurement

- [`images/ldr/01-ldr-voltage-divider-overview.jpg`](../images/ldr/01-ldr-voltage-divider-overview.jpg) — LDR divider implementation
- [`images/ldr/02-ldr-voltage-divider-close-up.jpg`](../images/ldr/02-ldr-voltage-divider-close-up.jpg) — midpoint wiring close-up
- [`images/ldr/03-ldr-uart-test-condition-a.png`](../images/ldr/03-ldr-uart-test-condition-a.png) — UART test condition A
- [`images/ldr/04-ldr-uart-test-condition-b.png`](../images/ldr/04-ldr-uart-test-condition-b.png) — UART test condition B

## 7. DHT11 Verification

- [`images/dht11/01-dht11-and-oled-running.jpg`](../images/dht11/01-dht11-and-oled-running.jpg) — DHT11 and OLED operating together
- [`images/dht11/04-dht11-uart-readings.png`](../images/dht11/04-dht11-uart-readings.png) — UART temperature/humidity readings

## 8. Relay Driver

- [`images/relay-driver/03-bc337-resistor-network-close-up.jpg`](../images/relay-driver/03-bc337-resistor-network-close-up.jpg) — BC337/resistor network
- [`images/relay-driver/06-relay-driver-integrated-view.jpg`](../images/relay-driver/06-relay-driver-integrated-view.jpg) — integrated relay-driver view
- [`images/relay-driver/07-relay-switching-test-frame.jpg`](../images/relay-driver/07-relay-switching-test-frame.jpg) — frame captured from relay-switching validation video

## 9. Power Subsystem and Noise Mitigation

- [`images/power-subsystem/10-power-subsystem-complete-view.jpg`](../images/power-subsystem/10-power-subsystem-complete-view.jpg) — complete power subsystem
- [`images/power-subsystem/12-470uf-capacitor-close-up.jpg`](../images/power-subsystem/12-470uf-capacitor-close-up.jpg) — bulk capacitor close-up
- [`images/power-subsystem/13-1n4007-flyback-diode-close-up.jpg`](../images/power-subsystem/13-1n4007-flyback-diode-close-up.jpg) — 1N4007 close-up
- [`images/power-subsystem/14-100nf-ceramic-capacitor-close-up.jpg`](../images/power-subsystem/14-100nf-ceramic-capacitor-close-up.jpg) — motor suppression capacitor close-up
- [`images/power-subsystem/17-common-ground-connection-close-up.jpg`](../images/power-subsystem/17-common-ground-connection-close-up.jpg) — common ground connection
- [`images/power-subsystem/18-power-subsystem-running-frame.jpg`](../images/power-subsystem/18-power-subsystem-running-frame.jpg) — power subsystem running, captured from video

## 10. Troubleshooting Evidence

- [`images/troubleshooting/02-pump-safety-off-behavior.png`](../images/troubleshooting/02-pump-safety-off-behavior.png) — safety timeout behavior
- [`images/troubleshooting/03-temperature-error-and-pump-cycles.png`](../images/troubleshooting/03-temperature-error-and-pump-cycles.png) — temperature/DHT failure during pump-related testing
- [`images/troubleshooting/05-soil-adc-instability-during-pump-operation.png`](../images/troubleshooting/05-soil-adc-instability-during-pump-operation.png) — ADC disturbance during pump operation
- [`images/troubleshooting/06-oled-display-corruption-close-up.png`](../images/troubleshooting/06-oled-display-corruption-close-up.png) — OLED corruption close-up
- [`images/troubleshooting/07-oled-display-corruption-after-pump-operation.png`](../images/troubleshooting/07-oled-display-corruption-after-pump-operation.png) — display corruption after pump operation

These images document why filtering, confirmation counters, power suppression components, grounding changes, and physical subsystem separation were introduced.

## 11. Component Reference Images

The `images/components/` directory contains individual reference photographs for the LaunchPad, OLED, DHT11, soil sensor, LDR, relay, BC337, LM2596, pump, diode, resistors, capacitors, and battery holder.

## 12. Video Tests

### Final demonstration

- [`videos/final-demo/01-final-irrigation-demo.mp4`](../videos/final-demo/01-final-irrigation-demo.mp4)

### Subsystem / integration recordings

- [`videos/subsystem-tests/01-soil-moisture-water-test.mp4`](../videos/subsystem-tests/01-soil-moisture-water-test.mp4)
- [`videos/subsystem-tests/02-oled-sensor-subsystem-test.mp4`](../videos/subsystem-tests/02-oled-sensor-subsystem-test.mp4)
- [`videos/subsystem-tests/03-relay-driver-switching-test.mp4`](../videos/subsystem-tests/03-relay-driver-switching-test.mp4)
- [`videos/subsystem-tests/04-pump-display-integration-test.mp4`](../videos/subsystem-tests/04-pump-display-integration-test.mp4)
- [`videos/subsystem-tests/05-sensitive-subsystem-soil-test.mp4`](../videos/subsystem-tests/05-sensitive-subsystem-soil-test.mp4)
- [`videos/subsystem-tests/06-integrated-system-control-test.mp4`](../videos/subsystem-tests/06-integrated-system-control-test.mp4)

The MP4 files are repository-optimized copies. The original 4K MOV recordings should remain in the local project archive.

## 13. Documentation Use

When a still image was extracted from a project video, it should be described in formal documentation as a **frame captured from the system validation video** rather than as a separately photographed test image.
