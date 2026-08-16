# Troubleshooting Coverage Audit

This file is a completeness check for the development record. It maps every engineering failure and repository-publication problem that was retained from the project history to the document that explains it, the correction that was applied, and the evidence or verification that remains in the repository.

The detailed engineering narratives are in [`troubleshooting.md`](troubleshooting.md). This audit exists so a reviewer can verify that the development history was not reduced to only the successful final build.

## A. Embedded-System Engineering Problems

| # | Problem encountered | Final correction | Where documented / verified |
|---|---|---|---|
| 1 | Soil ADC values existed but the percentage was not physically meaningful | Empirical dry/wet calibration using `4090` and `1650`, with clamped conversion | `troubleshooting.md` §1, `test-results.md`, `source/main.c`, `images/soil-sensor/` |
| 2 | A single soil threshold could make the pump chatter near the decision point | 35% start and 50% stop thresholds were separated to create hysteresis | `troubleshooting.md` §2, `source/main.c` |
| 3 | Relay input could not be treated as a reliable direct 3.3 V GPIO load | Added BC337 relay-input driver, 1 kΩ base resistor and 10 kΩ pull-up | `troubleshooting.md` §3, `hardware-connections.md`, `images/relay-driver/` |
| 4 | Early BC337 wiring did not switch correctly and incorrect configurations caused abnormal transistor behavior/heating | Rebuilt base/collector/emitter and resistor nodes using the verified topology | `troubleshooting.md` §4, `images/relay-driver/03-bc337-resistor-network-close-up.jpg` |
| 5 | Relay clicked but pump remained silent / did not follow intended normally-off behavior | Corrected power contacts to `OUT+ -> COM`, `NO -> Pump +`, `Pump - -> OUT-` | `troubleshooting.md` §5, `hardware-connections.md` |
| 6 | Runtime timeout was initially being treated too much like the normal irrigation stop mechanism | Defined confirmed wet soil as the normal stop and runtime only as a fail-safe | `troubleshooting.md` §6, `source/main.c` |
| 7 | Initial 10 s safety timeout was too short for realistic wetting | Finalized maximum continuous pump runtime at 20 s | `troubleshooting.md` §7, `source/main.c`, `images/uart/03-final-uart-monitoring-safety-state.png` |
| 8 | Pump could stop/restart because motor noise created abrupt soil readings | Added 16-sample ADC averaging plus 3 consecutive dry / 3 consecutive wet confirmation | `troubleshooting.md` §8, `source/main.c`, `images/troubleshooting/05-soil-adc-instability-during-pump-operation.png` |
| 9 | OLED text became corrupted during pump operation | Added motor suppression/power filtering, then physically separated sensitive and power circuitry and simplified grounding | `troubleshooting.md` §9, `images/troubleshooting/06-*`, `07-*`, `images/power-subsystem/` |
| 10 | DHT11 produced transient `TEMP: ERR` / invalid reads around pump operation | Added retries, retained the last valid reading and deferred DHT transactions while pump was active | `troubleshooting.md` §10, `source/main.c`, `images/troubleshooting/03-temperature-error-and-pump-cycles.png` |
| 11 | OLED froze on an old value while UART and the main control loop continued | Used UART to isolate the failure to OLED/I2C/power environment; final fix was hardware filtering/layout redesign | `troubleshooting.md` §11, `development-log.md` |
| 12 | Aggressive automatic I2C/OLED recovery experiment made behavior worse / did not solve the failure | Removed the recovery experiment and restored the simpler working I2C path; fixed the electrical root cause instead | `troubleshooting.md` §12 |
| 13 | Some OLED labels/characters were missing or incorrect in intermediate revisions | Restricted strings to supported glyphs or added required custom-font characters | `troubleshooting.md` §13, final OLED photos |
| 14 | A complete source implementation was accidentally pasted twice, producing duplicate functions and multiple `main()` definitions | Removed the duplicate half and rebuilt one complete source implementation | `troubleshooting.md` §14, clean `source/main.c` |
| 15 | One earlier syntax/parser problem caused a cascade of misleading compile errors | Fixed the earliest syntax/context problem first, then removed the real duplicate definitions | `troubleshooting.md` §15 |
| 16 | Prototype accumulated too many ground/return-current paths and OLED behavior changed when some were removed | Reworked grounding into a deliberate shared reference between separated subsystems | `troubleshooting.md` §16, `hardware-connections.md`, `images/power-subsystem/17-common-ground-connection-close-up.jpg` |
| 17 | Incremental breadboard became too complex to diagnose reliably | Rebuilt the system from scratch as a sensitive subsystem plus a power/pump subsystem | `troubleshooting.md` §17, `development-log.md`, final-system photos |
| 18 | Automatic irrigation started immediately after power-up, making controlled testing/demo behavior awkward | Added SW1/PF4 STOP/RUN toggle; startup now forces pump OFF and waits for a manual START | `troubleshooting.md` §18, `source/main.c`, `images/final-system/10-sw1-start-stop-control-frame.jpg` |
| 19 | Final stability could not be accepted from one subsystem test alone | Defined an integrated verification checklist covering OLED, UART, DHT, soil, LDR, relay, pump, wet/dry logic, timeout and SW1 | `troubleshooting.md` §19, `test-results.md`, final demo video |

## B. Final Control-Logic Cross-Check Against Source

The documentation values match the current final source:

```text
MOISTURE_LOW_THRESHOLD   = 35
MOISTURE_HIGH_THRESHOLD  = 50
MAX_PUMP_RUNTIME         = 20 s
ADC_SAMPLE_COUNT         = 16
DRY_CONFIRM_COUNT        = 3
WET_CONFIRM_COUNT        = 3
SOIL_DRY_VALUE           = 4090
SOIL_WET_VALUE           = 1650
DHT_READ_INTERVAL        = 2 s
DHT_MAX_RETRIES          = 3
```

The final application also verifies the following implementation details:

- pump GPIO is forced OFF before normal application operation
- SW1 is active-low through PF4 internal pull-up
- one press enters RUN and resets pump/safety/confirmation state
- a second press returns to STOP and forces the pump OFF
- safety timeout latches `SAFETY OFF` until controller state is reset
- DHT11 reads are skipped while the pump is running and last-valid data is retained
- UART continues as an independent diagnostic path

## C. Repository / Publication Problems

These were not embedded-control failures, but they were encountered while turning the finished project into a reproducible GitHub repository and are recorded here so the project history is complete.

| # | Publication problem | Resolution |
|---|---|---|
| P1 | Original phone photographs were HEIC and were inconvenient for direct GitHub/Markdown use | Curated copies were converted to JPG/PNG; `.gitignore` excludes HEIC originals from the repository |
| P2 | GitHub browser upload rejected media above the web upload limit | GitHub Desktop was used to push the repository; optimized MP4 files were kept below Git's normal single-file limit |
| P3 | Original 4K video files were unnecessarily large for the repository | Repository copies were encoded as H.264/MP4; original high-resolution recordings remain in the local archive |
| P4 | Windows hidden file extensions caused the readable source to be published once as `main.c.c` | Enabled file-name extensions, renamed it to exactly `source/main.c`, committed and pushed the correction |
| P5 | CCS project keeps the tested application filename `hello.c`, while a public repository benefits from `main.c` | Preserved `firmware/irrigation_system/hello.c` for CCS-project consistency and added identical `source/main.c` as the clean readable source; this is explained in `build-and-run.md` |
| P6 | Generated IDE/build output and local editor metadata would add noise to version control | `Debug/`, `Release/`, object/binary outputs, HEIC files, OS junk and selected local metadata are excluded through `.gitignore`; unnecessary local metadata was removed |
| P7 | GitHub Desktop reported LF/CRLF conversion warnings on project metadata | Added `.gitattributes` to define text/binary handling and keep line-ending behavior predictable |

## D. Evidence Coverage

The repository contains evidence for both success and failure states, not only final glamour shots:

- final system and subsystem photographs
- component reference photographs
- dry/wet/safety UART screenshots
- DHT11 error evidence
- soil ADC instability evidence
- OLED corruption evidence
- relay-driver and power-path close-ups
- common-ground close-up
- diode, 100 nF and 470 µF mitigation-component close-ups
- final demonstration video
- six subsystem/integration test videos

A complete file-by-file media inventory is maintained in [`media-inventory.md`](media-inventory.md). The smaller claim-oriented evidence guide is [`media-evidence.md`](media-evidence.md).

## E. Audit Result

The current repository preserves the major hardware, firmware, integration, diagnostic and publication problems that materially affected the project. Each engineering problem above has a recorded symptom, cause or diagnostic interpretation, implemented correction, and final verification path. No known project-critical failure from the retained development history is intentionally omitted.
