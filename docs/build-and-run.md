# Build and Run Guide

This guide explains how to open, build, flash, and verify the TM4C123 Smart Irrigation System from the repository.

## 1. Requirements

The project was developed with the following environment:

- Code Composer Studio
- TivaWare for C Series 2.2.0.295
- TI ARM Compiler 20.2.7.LTS
- EK-TM4C123GXL Tiva C LaunchPad
- Stellaris ICDI drivers
- PuTTY or another serial terminal

The repository contains both a readable application source file and the complete CCS project metadata used for the working firmware.

## 2. Repository Files

```text
source/main.c
```

contains the final readable application source.

```text
firmware/irrigation_system/
```

contains the Code Composer Studio project, including:

```text
.ccsproject
.cproject
.project
.settings/
targetConfigs/
hello.c
hello_ccs.cmd
startup_ccs.c
target_config.ccxml
```

`firmware/irrigation_system/hello.c` and `source/main.c` contain the same final application code. The original CCS source filename is kept as `hello.c` so the imported project remains consistent with the project metadata that produced the tested build.

## 3. TivaWare Location

The imported CCS project was originally created from a TivaWare example and contains references to the local TivaWare installation.

The development machine used:

```text
C:/ti/TivaWare_C_Series-2.2.0.295/
```

If TivaWare is installed in another location, update the project include/library paths in CCS before building.

## 4. Import the CCS Project

In Code Composer Studio:

```text
File
-> Import
-> CCS Projects
```

Select the repository folder:

```text
firmware/irrigation_system/
```

Allow CCS to detect and import the existing project metadata.

Do not rename the CCS project files or `hello.c` before importing. The separate `source/main.c` file exists specifically to provide a clean repository-facing filename without changing the tested CCS project structure.

## 5. Connect the LaunchPad

Connect the EK-TM4C123GXL to the computer through the LaunchPad debug USB connector.

Verify that:

- the LaunchPad is detected by the Stellaris ICDI driver
- the correct target configuration is selected
- no other application is holding the debug interface

## 6. Build

In CCS, select the imported project and use:

```text
Project -> Build Project
```

or the standard build shortcut.

The repository intentionally excludes generated `Debug/`, object files, binaries, maps, and other build outputs. CCS will regenerate them locally.

## 7. Flash and Run

After a successful build:

```text
Run -> Debug
```

or use the CCS debug/flash command appropriate for the installed version.

The firmware enters STOP mode after startup. Sensor monitoring and display/UART output continue, but automatic watering remains disabled until SW1 is pressed.

## 8. SW1 Operation

SW1 is the LaunchPad PF4 user button and is active-low through the internal pull-up.

```text
Power-up     -> System STOP / Pump OFF
First press  -> System RUN / automatic irrigation enabled
Second press -> System STOP / pump forced OFF / state reset
```

A short software debounce delay and edge detection prevent a held button from repeatedly toggling the state.

## 9. UART Monitoring

Connect to the LaunchPad virtual serial port with:

```text
Baud rate:    115200
Data bits:    8
Parity:       None
Stop bits:    1
Flow control: None
```

A normal status line has the form:

```text
ADC:2695 | Soil:57% | Light:23% | System:RUN | Pump:OFF | Time:0s | Temp:29C | Hum:32%
```

The UART output is the primary diagnostic channel for verifying sensor values, control state, pump runtime, and DHT11 status independently of the OLED.

## 10. Expected Control Behavior

The final control constants are:

```text
Dry threshold:       Soil < 35%
Wet threshold:       Soil > 50%
Dry confirmation:    3 samples
Wet confirmation:    3 samples
ADC averaging:       16 samples
Maximum pump runtime: 20 s
```

Expected sequence:

```text
SW1 START
-> 3 consecutive dry measurements
-> Pump ON
-> soil moisture rises
-> 3 consecutive wet measurements
-> Pump OFF
```

If the pump reaches 20 seconds without satisfying the normal wet-stop condition:

```text
Pump -> SAFETY OFF
```

The safety lock is cleared by the controller reset/STOP cycle defined by the firmware.

## 11. Hardware Verification Before Power-Up

Before running the complete system, verify the final wiring in `docs/hardware-connections.md`.

Important checks:

- relay contact path uses COM and NO
- pump polarity is correct
- BC337 base is driven through the 1 kΩ resistor
- relay-input pull-up uses the 10 kΩ resistor
- 470 µF capacitor polarity is correct
- 1N4007 stripe is on the pump-positive side
- 100 nF ceramic capacitor is across the pump terminals
- sensitive and power subsystems share the required common ground reference

## 12. Project Media

Validation photographs, UART screenshots, troubleshooting evidence, and subsystem test recordings are indexed in:

```text
docs/media-evidence.md
```

The final demonstration is stored at:

```text
videos/final-demo/01-final-irrigation-demo.mp4
```
