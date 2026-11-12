/* button.c on a PC, with made-up press timings. From firmware/stm32:
 *
 *   gcc -Wall -ICore/Inc Core/Src/button.c test/test_button.c -o test_button.exe
 *   ./test_button.exe
 */
#include <assert.h>
#include <stdio.h>

#include "button.h"

static uint32_t now = 1000;

/* Holds the level for ms milliseconds, stepping 1 ms like the main loop, and returns the one
 * event it gave (BUTTON_NONE if none). Fails the test if it gave more than one. */
static button_event_t hold(int pressed, uint32_t ms)
{
  button_event_t seen = BUTTON_NONE;
  for (uint32_t t = 0; t < ms; t++, now++)
  {
    button_event_t e = button_update(pressed, now);
    if (e != BUTTON_NONE)
    {
      assert(seen == BUTTON_NONE);
      seen = e;
    }
  }
  return seen;
}

/* A press of held_ms, then up_ms released: the events at the press and at the release. */
static void press(uint32_t held_ms, uint32_t up_ms, button_event_t *down, button_event_t *up)
{
  *down = hold(1, held_ms);
  *up = hold(0, up_ms);
}

static void test_gesture_hold(void)
{
  button_event_t down, up;
  press(600, 1000, &down, &up);
  assert(down == BUTTON_PRESSED);
  assert(up == BUTTON_GESTURE);
  assert(!button_held());
}

static void test_press_is_seen_at_once(void)
{
  // ml/data/ was recorded from the first edge, so recording must start on it
  assert(button_update(1, now) == BUTTON_PRESSED);
  assert(button_held());
  hold(1, 500);
  assert(hold(0, 1000) == BUTTON_GESTURE);
}

static void test_bounce_is_ignored(void)
{
  // contacts chatter for 5 ms on press and release
  assert(button_update(1, now++) == BUTTON_PRESSED);
  for (int i = 0; i < 5; i++)
    assert(button_update(i % 2, now++) == BUTTON_NONE);
  assert(hold(1, 600) == BUTTON_NONE);
  assert(button_update(0, now++) == BUTTON_GESTURE);
  for (int i = 0; i < 5; i++)
    assert(button_update(!(i % 2), now++) == BUTTON_NONE);
  assert(hold(0, 1000) == BUTTON_NONE);
}

static void test_double_tap(void)
{
  button_event_t down, up;
  press(80, 150, &down, &up);
  assert(down == BUTTON_PRESSED && up == BUTTON_NONE);
  press(80, 1000, &down, &up);
  assert(down == BUTTON_PRESSED && up == BUTTON_DOUBLE_TAP);
}

static void test_single_tap_does_nothing(void)
{
  button_event_t down, up;
  press(80, 1000, &down, &up);
  assert(down == BUTTON_PRESSED && up == BUTTON_NONE);
}

static void test_taps_too_far_apart(void)
{
  button_event_t down, up;
  press(80, BUTTON_DOUBLE_TAP_MS + 50, &down, &up);
  assert(up == BUTTON_NONE);
  press(80, 1000, &down, &up);
  assert(up == BUTTON_NONE);  // only the first tap of a new pair
}

static void test_triple_tap_is_one_double_tap(void)
{
  button_event_t down, up;
  press(80, 150, &down, &up);
  press(80, 150, &down, &up);
  assert(up == BUTTON_DOUBLE_TAP);
  press(80, 1000, &down, &up);
  assert(up == BUTTON_NONE);
}

static void test_tap_then_gesture(void)
{
  // a stray tap just before a gesture doesn't pair with the gesture or with a tap after it
  button_event_t down, up;
  press(80, 150, &down, &up);
  press(600, 150, &down, &up);
  assert(up == BUTTON_GESTURE);
  press(80, 1000, &down, &up);
  assert(up == BUTTON_NONE);
}

static void test_tap_limit(void)
{
  button_event_t down, up;
  press(BUTTON_TAP_MAX_MS - 1, 1000, &down, &up);
  assert(up == BUTTON_NONE);
  press(BUTTON_TAP_MAX_MS, 1000, &down, &up);
  assert(up == BUTTON_GESTURE);
}

static void test_tick_wraps(void)
{
  now = UINT32_MAX - 300;
  hold(0, 100);
  button_event_t down, up;
  press(600, 1000, &down, &up);  // crosses 0 while held
  assert(down == BUTTON_PRESSED && up == BUTTON_GESTURE);
  now = UINT32_MAX - 200;
  hold(0, 100);
  press(80, 150, &down, &up);    // crosses 0 between the taps
  press(80, 1000, &down, &up);
  assert(up == BUTTON_DOUBLE_TAP);
}

int main(void)
{
  hold(0, 100);  // settle after boot
  test_gesture_hold();
  test_press_is_seen_at_once();
  test_bounce_is_ignored();
  test_double_tap();
  test_single_tap_does_nothing();
  test_taps_too_far_apart();
  test_triple_tap_is_one_double_tap();
  test_tap_then_gesture();
  test_tap_limit();
  test_tick_wraps();
  printf("button: all tests passed\n");
  return 0;
}
