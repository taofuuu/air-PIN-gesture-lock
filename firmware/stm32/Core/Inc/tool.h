/* Bench firmware: the lock's setup in main.c, then a loop of its own instead of the lock's.
 *
 * The build configuration picks one by defining its symbol:
 *   Collect      TOOL_COLLECT       tool_collect.c       streams samples for ml/collect.py
 *   GestureTest  TOOL_GESTURE_TEST  tool_gesture_test.c  times the model on one button hold
 *   HwTest       TOOL_HW_TEST       tool_hw_test.c       a click opens the relay, no model
 * Debug and Release define none of them and build the lock. */
#ifndef TOOL_H
#define TOOL_H

#if defined(TOOL_COLLECT) || defined(TOOL_GESTURE_TEST) || defined(TOOL_HW_TEST)
#define TOOL
#endif

void tool_run(void);  // never returns

#endif /* TOOL_H */
