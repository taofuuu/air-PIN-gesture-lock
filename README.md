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

## Wiring

Pins are fixed in the CubeMX `.ioc`; keep this table in sync with it.

| Part | Pin | Nucleo pin |
|------|-----|------------|
| MPU-6050 | SDA / SCL | TBD (I2C) |
| MPU-6050 | VCC / GND | 3V3 / GND |
| KY-004 | S | TBD (GPIO input) |
| KY-019 | S | TBD (GPIO output) |
| KY-019 | + / - | 5V / GND |

> Power the solenoid from its own 5V supply through the relay's COM/NO contacts, not from the
> Nucleo's 5V pin, and put a flyback diode across the solenoid. Share GND with the Nucleo.

## Project layout

The STM32CubeIDE project lives in this folder. Create it with
*File → New → STM32 Project*, board **NUCLEO-F411RE**, location set to this repo.
Build output and machine-local IDE files are ignored by `.gitignore`.

## Build and flash

Open the project in STM32CubeIDE, build (Ctrl+B), then *Run → Debug* to flash over the
on-board ST-LINK.
