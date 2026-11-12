/* Collect build configuration: one line per sample at 100 Hz for ml/collect.py,
 *   ax,ay,az,gx,gy,gz,btn
 * where btn is 1 while the KY-004 is held. The line format is private to this file and collect.py. */
#include "tool.h"

#ifdef TOOL_COLLECT
#include <stdio.h>
#include "imu.h"
#include "main.h"

#define SAMPLE_PERIOD_MS 10  // 100 Hz, matches the MPU-6050 sample rate set in imu.c

void tool_run(void)
{
  uint32_t next_sample = HAL_GetTick();
  imu_sample_t s;

  while (1)
  {
    // Signed difference keeps working when the tick counter wraps around
    if ((int32_t)(HAL_GetTick() - next_sample) >= 0)
    {
      next_sample += SAMPLE_PERIOD_MS;
      // KY-004 S is pulled up and reads low while pressed, as in the lock
      int held = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET;
      if (imu_read(&s) == 0)
        printf("%d,%d,%d,%d,%d,%d,%d\r\n", s.ax, s.ay, s.az, s.gx, s.gy, s.gz, held);
    }
  }
}
#endif /* TOOL_COLLECT */
