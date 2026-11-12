# Air PIN Gesture Lock

Embedded systems lab project: a door lock you open by "drawing" a PIN in the air.
Press the button, move the MPU-6050 through a sequence of gestures, and if the
sequence matches the stored PIN the relay energizes the solenoid and the lock opens.

## Hardware

| Part | Role |
|------|------|
| STM32 Nucleo-F411RE | Main controller |
| MPU-6050 (I2C) | Accelerometer/gyro that captures the gestures |
| KY-004 push button | Start / confirm PIN entry |
| KY-019 5V relay module | Switches the solenoid supply |
| 5V solenoid lock | The lock itself |
| ESP32 (UART) | Wi-Fi and the web IoT dashboard |

## Wiring

Pins are fixed in the CubeMX `.ioc`; keep this table in sync with it.
[hardware/wiring.svg](hardware/wiring.svg) draws the sensor and button wiring.

| Part | Pin | Nucleo pin |
|------|-----|------------|
| MPU-6050 | SDA / SCL | D14 (PB9) / D15 (PB8), I2C1 |
| MPU-6050 | VCC / GND | 3V3 / GND |
| KY-004 | S | D4 (PB5), GPIO input with pull-up, low while pressed |
| KY-004 | + (middle pin) / - | 3V3 / GND |
| KY-019 | S | D7 (PA8), GPIO output; only the HwTest firmware sets it up so far, not the `.ioc` |
| KY-019 | + / - | 5V / GND |
| ESP32 | RX / TX / GND | TBD (UART TX / RX, not USART2) / GND |

> Power the solenoid from its own 5V supply through the relay's COM/NO contacts, not from the
> Nucleo's 5V pin, and put a flyback diode across the solenoid. Share GND with the Nucleo.

## Project layout

See [STRUCTURE.md](STRUCTURE.md) for the folders, who owns which part, and the interfaces
between the parts.

The STM32CubeIDE project lives in `firmware/stm32/`. Open it with
*File → Import → General → Existing Projects into Workspace* and pick `firmware/stm32` in this
repo, leaving *Copy projects into workspace* unticked.
Build output and machine-local IDE files are ignored by `.gitignore`.

## Build and flash

The one project builds several firmwares. Pick one from the arrow next to the hammer
(*Project → Build Configurations → Set Active*), build (Ctrl+B), then *Run → Debug* to flash over
the on-board ST-LINK.

| Build configuration | Firmware |
|---------------------|----------|
| Debug, Release | the lock |
| Collect | streams samples and the button for `ml/collect.py` |
| GestureTest | hold, gesture, release: prints the gesture ID and how long the model took |
| HwTest | no model: a button click opens the relay for 3 s, and the gyro and button are printed |

Each configuration builds into its own folder (`Debug/`, `Collect/`, ...), so the debug launch
must point at that folder's `.elf`.
