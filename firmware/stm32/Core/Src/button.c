#include "button.h"

static int held;
static uint32_t last_change;  // when held last changed
static uint32_t press_start;
static int tap_pending;       // a tap was released and could still become a double tap
static uint32_t tap_released;

button_event_t button_update(int pressed, uint32_t now_ms)
{
  pressed = pressed != 0;
  // Take a change at once, then ignore the bounce after it. Waiting for the level to settle
  // instead would start every recording late, and ml/data/ was recorded from the first edge.
  if (pressed == held || now_ms - last_change < BUTTON_DEBOUNCE_MS)
    return BUTTON_NONE;
  held = pressed;
  last_change = now_ms;

  if (held)
  {
    press_start = now_ms;
    return BUTTON_PRESSED;
  }

  if (now_ms - press_start >= BUTTON_TAP_MAX_MS)
  {
    tap_pending = 0;
    return BUTTON_GESTURE;
  }
  if (tap_pending && press_start - tap_released <= BUTTON_DOUBLE_TAP_MS)
  {
    tap_pending = 0;
    return BUTTON_DOUBLE_TAP;
  }
  tap_pending = 1;
  tap_released = now_ms;
  return BUTTON_NONE;
}

int button_held(void)
{
  return held;
}
