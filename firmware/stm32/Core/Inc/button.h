/* KY-004 button: one hold = one gesture, a quick double tap = clear.
 *
 * Pure logic with no HAL calls, so test/test_button.c can run it on a PC. main.c feeds it the pin
 * level and HAL_GetTick() on every loop and acts on the events. */
#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

#define BUTTON_DEBOUNCE_MS   20   // a change within this long of the last one is contact bounce
#define BUTTON_TAP_MAX_MS    200  // shorter holds are taps; the shortest gesture in ml/data/ is 400 ms
#define BUTTON_DOUBLE_TAP_MS 400  // most time from releasing the first tap to pressing the second

typedef enum
{
  BUTTON_NONE,
  BUTTON_PRESSED,     // start recording
  BUTTON_GESTURE,     // released after a hold long enough to be a gesture: classify the recording
  BUTTON_DOUBLE_TAP,  // second of two quick taps
} button_event_t;

/* pressed is the pin's level already turned into "held down". Call it at least every few ms. */
button_event_t button_update(int pressed, uint32_t now_ms);

/* Whether the button is held, after debouncing. */
int button_held(void);

#endif /* BUTTON_H */
