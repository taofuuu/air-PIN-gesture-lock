/* HwTest build configuration: the lock without the model. One button click opens the relay for
 * UNLOCK_MS where the lock would classify gestures and check the PIN. Prints the gyro, the button
 * and the lock state 10 times a second.
 * The relay pin is set up here, not in the .ioc, until the lock drives the relay too. */
#include "tool.h"

#ifdef TOOL_HW_TEST
#include <stdio.h>
#include "imu.h"
#include "main.h"

#define PRINT_PERIOD_MS 100   // 10 lines a second, slow enough to read in a terminal
#define DEBOUNCE_MS     20    // the button must hold steady this long to count
#define UNLOCK_MS       3000  // most 5V solenoid locks only take short pulses

#define RELAY_GPIO_Port GPIOA
#define RELAY_Pin       GPIO_PIN_8      // D7, free of the USARTs the ESP32 could take
#define RELAY_ON        GPIO_PIN_SET    // KY-019 triggers on high: swap the two if yours is inverted
#define RELAY_OFF       GPIO_PIN_RESET

static int btn_down(void)
{
  // KY-004 S is pulled up and reads low while pressed, as in the lock
  return HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET;
}

void tool_run(void)
{
  GPIO_InitTypeDef gpio = {0};
  uint32_t next_print, now, btn_changed = 0, relock_at = 0;
  imu_sample_t s;
  int raw, last_raw, pressed, unlocked = 0;

  HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, RELAY_OFF);  // before the pin becomes an output
  gpio.Pin = RELAY_Pin;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RELAY_GPIO_Port, &gpio);

  next_print = HAL_GetTick();
  pressed = last_raw = btn_down();  // so a button reading pressed at reset can't open the lock

  while (1)
  {
    now = HAL_GetTick();

    raw = btn_down();
    if (raw != last_raw)
    {
      last_raw = raw;
      btn_changed = now;
    }
    if (raw != pressed && now - btn_changed >= DEBOUNCE_MS)
    {
      pressed = raw;
      // The lock decides here: gesture_classify(), then pin_add() == PIN_OK.
      // Ignoring clicks while open keeps the solenoid's on time to one UNLOCK_MS.
      if (pressed && !unlocked)
      {
        unlocked = 1;
        relock_at = now + UNLOCK_MS;
        HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, RELAY_ON);
      }
    }

    // Signed differences keep working when the tick counter wraps around
    if (unlocked && (int32_t)(now - relock_at) >= 0)
    {
      unlocked = 0;
      HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, RELAY_OFF);
    }

    if ((int32_t)(now - next_print) >= 0)
    {
      next_print += PRINT_PERIOD_MS;
      if (imu_read(&s) == 0)
        printf("gx=%6d gy=%6d gz=%6d  ", s.gx, s.gy, s.gz);  // raw counts, 65.5 = 1 deg/s
      else
        printf("IMU READ FAILED  ");
      printf("btn=%d lock=%s\r\n", pressed, unlocked ? "OPEN" : "LOCKED");
    }
  }
}
#endif /* TOOL_HW_TEST */
