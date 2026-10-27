# Repo structure

Three people, three parts. Each part builds and tests on its own and talks to the others
only through the interfaces at the bottom of this file. Change an interface only after
all three agree.

| Part | Owner | Branch |
|------|-------|--------|
| ML gesture model | taofuuu | `ml-model` |
| PIN mapping | TBD | `pin-mapping` |
| ESP32 + web IoT dashboard | TBD | `esp32-dashboard` |

## Layout

```
air-PIN-gesture-lock/
├── README.md                 overview, wiring, how to build and flash
├── STRUCTURE.md              this file
├── hardware/
│   └── wand.scad             3D-printed wand handle
├── firmware/
│   ├── stm32/                STM32CubeIDE project, NUCLEO-F411RE          shared
│   │   ├── *.ioc             CubeMX pins and peripherals                 shared
│   │   └── Core/
│   │       ├── Inc/          one header per module = the interfaces below
│   │       └── Src/
│   │           ├── main.c    init, button, relay, calls the modules      shared
│   │           ├── imu.c     MPU-6050 driver                             shared
│   │           ├── gesture.c runs the trained model on a recording       ML
│   │           ├── pin.c     gesture sequence -> PIN check               PIN mapping
│   │           └── link.c    UART messages to and from the ESP32         ESP32
│   └── esp32/                ESP32 project: Wi-Fi, web server            ESP32
│       └── data/             dashboard HTML/CSS/JS served by the ESP32   ESP32
└── ml/                       training on the PC                          ML
    ├── collect.py            records labelled gestures from the Nucleo's USB serial
    ├── train.py              trains, evaluates and exports the model
    ├── data/                 recorded gestures, one CSV per recording
    └── model/                exported model that gesture.c is generated from
```

The model runs on the STM32 because the sensor is wired there. Code generated from
`ml/model/` (X-CUBE-AI or TensorFlow Lite Micro) goes into `firmware/stm32/` and is
regenerated, never edited by hand.

## Interfaces

**1. IMU to ML** (`imu.h`)

```c
typedef struct { int16_t ax, ay, az, gx, gy, gz; } imu_sample_t;  // raw MPU-6050 counts
```

Sample rate and full-scale ranges are fixed in `imu.c`. Training data must be recorded with
the same settings, so changing them means re-recording.

**2. ML to PIN mapping** (`gesture.h`)

```c
typedef enum { GESTURE_UP, GESTURE_DOWN, GESTURE_LEFT, GESTURE_RIGHT, GESTURE_COUNT } gesture_t;
int gesture_classify(const imu_sample_t *samples, size_t n);  // gesture_t, or -1 if unsure
```

One button hold = one gesture: recording starts when the button is pressed and stops when it
is released. That saves the model from having to find where one gesture ends and the next begins.

**3. PIN mapping to main** (`pin.h`)

```c
typedef enum { PIN_INCOMPLETE, PIN_OK, PIN_WRONG } pin_result_t;
void pin_reset(void);
pin_result_t pin_add(int gesture);  // main energises the relay on PIN_OK
```

**4. STM32 to ESP32** (`link.h`, UART 115200 8N1, one text line per message)

```
STM32 -> ESP32:  GESTURE <id>    UNLOCK OK    UNLOCK FAIL
ESP32 -> STM32:  OPEN            (remote unlock from the dashboard)
```

USART2 is the ST-LINK USB serial and stays free for `ml/collect.py` and debug prints, so the
ESP32 uses another UART. Both boards are 3.3 V logic; connect TX to RX, RX to TX and GND.

## Working rules

- Work on your own branch and merge to `main` through a pull request.
- Edit only your own files. Shared files (`main.c`, `imu.c`, `*.ioc`, headers in `Inc/`)
  change in small PRs that the others review.
- In CubeMX-generated files, write code only between `/* USER CODE BEGIN */` and
  `/* USER CODE END */`; regenerating wipes everything else.
- Test without the others:
  - ML: train and evaluate on the PC from `ml/data/`.
  - PIN mapping: feed `pin_add()` fake gesture ids.
  - ESP32: type the line protocol into a USB-serial terminal instead of a real STM32.
