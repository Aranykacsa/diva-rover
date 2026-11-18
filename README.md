# Rover Simple Servo Drive

Differential drive continuous-rotation servo motor driver for **ESP32-S3** built with **ESP-IDF** (v5.x) and **PlatformIO**.

This project provides a hardware-level PWM servo driver using the ESP32-S3 LEDC peripheral to control differential-drive rovers powered by Parallax #900-00008 continuous rotation servos.

---

## Hardware Requirements

- **Microcontroller:** ESP32-S3 (ESP32-S3-DevKitC-1 or compatible)
- **Servos:** [Parallax #900-00008](docs/900-00008-Continuous-Rotation-Servo-Documentation--273045.pdf) Continuous Rotation Servos (2x or 4x paired)
- **Power Supply:** External 4.0V–6.0V DC supply (battery pack or bench supply rated for servo currents)

### Wiring Diagram & Pinout

| ESP32-S3 Pin | Servo Connection | Signal / Purpose |
| :--- | :--- | :--- |
| **GPIO 4** | Left Servo Signal (White / Orange wire) | LEDC Channel 0 (50 Hz PWM) |
| **GPIO 5** | Right Servo Signal (White / Orange wire) | LEDC Channel 1 (50 Hz PWM) |
| **GND** | Servo GND (Black / Brown wire) | **Common Ground** (Tie ESP32 GND to External PSU GND) |
| *Do NOT connect* | Servo V+ (Red wire) | Connect **only** to external 4–6V DC power supply |

> [!CAUTION]
> **Never power servos directly from the ESP32 dev board (3.3V or 5V pins).**  
> Servos draw significant current peaks and stall currents that can brown out or permanently damage the microcontroller's voltage regulator. Connect the servo power wire ($V_{servo}$) to a dedicated external power supply, and ensure external supply ground is tied to ESP32 ground.

---

## Servo Timing & Characteristics

According to the Parallax #900-00008 datasheet:

- **PWM Frequency:** 50 Hz (20 ms period)
- **LEDC Duty Resolution:** 14-bit (~1.22 µs / step)
- **Pulse Width Range:**
  - `1500 µs`: Neutral / Center (No rotation)
  - `1300 µs`: Full speed Clockwise (CW)
  - `1700 µs`: Full speed Counter-Clockwise (CCW)
  - Operating half-range: $\pm 200\ \mu\text{s}$

Once configured via `servo_pwm_attach()`, the ESP32-S3 LEDC hardware timer maintains the 50 Hz waveform continuously in hardware with **zero ongoing CPU overhead**.

---

## Configuration & Tuning

All hardware mappings and motor parameters are centralized in [`include/rover_config.h`](include/rover_config.h):

```c
/* Pin assignment */
#define ROVER_LEFT_SERVO_GPIO   GPIO_NUM_4
#define ROVER_RIGHT_SERVO_GPIO  GPIO_NUM_5

/* Timing specifications */
#define ROVER_SERVO_PERIOD_US      20000u   /* 50 Hz refresh */
#define ROVER_SERVO_NEUTRAL_US     1500u    /* Stop / neutral point */
#define ROVER_SERVO_HALF_RANGE_US   200u    /* +-200 us full speed deviation */

/* Software trim (microseconds offset added to neutral) */
#define ROVER_LEFT_TRIM_US    0
#define ROVER_RIGHT_TRIM_US   (-20)

/* Differential mounting inversion */
#define ROVER_LEFT_INVERT   (-1)
#define ROVER_RIGHT_INVERT  (+1)
```

### Motor Tuning Guide

1. **Mechanical Zeroing:** Before adjusting software trim, zero each servo by supplying a `1500 µs` neutral pulse and adjusting the mechanical potentiometer on the back of the servo with a screwdriver until rotation completely stops.
2. **Mounting Inversion:** Because continuous-rotation servos face in opposite directions on differential chassis, one wheel must rotate CW while the other rotates CCW to move forward. The defaults (`ROVER_LEFT_INVERT = -1`, `ROVER_RIGHT_INVERT = +1`) accommodate mirrored mounting.
3. **Software Speed Trim:** If the rover veers to one side at full throttle due to minor manufacturing variations between servo motors, adjust `ROVER_RIGHT_TRIM_US` or `ROVER_LEFT_TRIM_US` in 5–10 µs increments.

---

## Project Structure

```text
├── CMakeLists.txt              # Top-level ESP-IDF CMake build script
├── platformio.ini              # PlatformIO environment configuration
├── sdkconfig.defaults          # ESP-IDF Kconfig overrides (USB-Serial/JTAG console)
├── docs/                       # Component datasheets
│   └── 900-00008-Continuous-Rotation-Servo-Documentation--273045.pdf
├── include/
│   └── rover_config.h          # Pinout, pulse widths, trim, and polarity configuration
└── src/
    ├── CMakeLists.txt          # Component registration for ESP-IDF
    ├── main.c                  # Application entry point & drive test
    ├── servo_pwm.c             # Low-level LEDC PWM driver implementation
    └── servo_pwm.h             # Servo PWM public API
```

---

## Build and Flash

### Option A: Using PlatformIO (Recommended)

1. Connect the ESP32-S3 via its USB-C port.
2. Build firmware:
   ```bash
   pio run
   ```
3. Flash the board:
   ```bash
   pio run -t upload
   ```
4. Open the serial monitor:
   ```bash
   pio device monitor
   ```

### Option B: Using ESP-IDF CLI

1. Set up the ESP-IDF environment (`. $IDF_PATH/export.sh`).
2. Target ESP32-S3:
   ```bash
   idf.py set-target esp32s3
   ```
3. Build, flash, and monitor:
   ```bash
   idf.py build flash monitor
   ```

> [!NOTE]
> The ESP32-S3-DevKitC-1 board communicates via native USB-Serial/JTAG (typically mounted at `/dev/ttyACM0` on Linux). `sdkconfig.defaults` automatically routes `ESP_LOG` console output to `CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y`.

---

## Operating Warnings

> [!WARNING]
> The current `main.c` is configured as a **direct hardware test**:
> - It drives both servos to 100% full forward speed immediately upon boot and holds duty cycle indefinitely.
> - There is no safety watchdog, ramp-up rate limiter, or software emergency stop in this test mode.
> - **Bench Test Recommendation:** Keep wheels elevated off the work surface or provide an accessible power cut-off switch before applying motor power.
