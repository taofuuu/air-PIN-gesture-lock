/* GestureTest build configuration: hold the KY-004, make a gesture and release. Prints the gesture
 * ID, how many samples the hold gave and how long gesture_classify() took. */
#include "tool.h"

#ifdef TOOL_GESTURE_TEST
#include <stdio.h>
#include "gesture.h"
#include "imu.h"
#include "main.h"

#define SAMPLE_PERIOD_MS 10  // 100 Hz, matches the MPU-6050 sample rate set in imu.c

static imu_sample_t recording[GESTURE_MODEL_SAMPLES];

void tool_run(void)
{
  uint32_t next_sample = HAL_GetTick();
  size_t recorded = 0;
  imu_sample_t s;

  while (1)
  {
    // Signed difference keeps working when the tick counter wraps around
    if ((int32_t)(HAL_GetTick() - next_sample) >= 0)
    {
      next_sample += SAMPLE_PERIOD_MS;
      // KY-004 S is pulled up and reads low while pressed, as in the lock
      int held = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET;
      if (imu_read(&s) == 0 && held && recorded < GESTURE_MODEL_SAMPLES)
        recording[recorded++] = s;
      if (!held && recorded > 0)
      {
        uint32_t start = HAL_GetTick();
        int gesture = gesture_classify(recording, recorded);
        printf("gesture %d, %u samples, %lu ms\r\n", gesture, (unsigned)recorded,
               (unsigned long)(HAL_GetTick() - start));
        recorded = 0;
        next_sample = HAL_GetTick();  // don't rush to catch up on samples missed while classifying
      }
    }
  }
}
#endif /* TOOL_GESTURE_TEST */
