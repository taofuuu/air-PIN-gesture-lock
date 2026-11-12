---
Status: accepted
---

# Unsure gestures are never PIN input

[ADR 0001](0001-classifier-first-embedding-later.md) fixes that `-1` means unsure and that every
caller handles it: the classifier returns it below a confidence threshold, and the embedding model
will return it when no template is close enough. `gesture.c` returns it in three cases today — no
samples or a model that failed to init, a wiggle that `one_movement()` will not accept, and a best
class below the threshold. All three mean *I could not read that*. `-1` is an abstain, not another
gesture. This ADR says what handling it means for `pin.c` and `main.c`.

1. **`-1` is never stored.** While recording a PIN, reject the gesture, ask for it again and store
   nothing, so a stored PIN holds only IDs the model actually returned. After three unsure in a
   row, abandon enrolment and keep the old PIN; a half-written PIN in flash locks everyone out.
2. **While entering, `-1` is neither a digit nor a wrong digit.** The slot stays unfilled and the
   user is asked to repeat, with feedback distinct from a wrong PIN. The test is `gesture < 0`, per
   rule 1 of ADR 0001, and it runs before any comparison with the stored digit.
3. **A run of five unsure is a fault, not a clumsy user.** `main.c` counts the run and asks the
   user to clear with two quick taps. It does not reset by itself.
4. **An entry with no accepted gesture for 20-30 s clears itself.**

```c
/* pin.c, above any comparison with the stored digit */
if (gesture < 0) return PIN_INCOMPLETE;  // unsure: ask again, no slot used

/* main.c, in finish_gesture() */
unsure = g < 0 ? unsure + 1 : 0;
if (unsure >= 5) printf("STUCK\r\n");    // run of unsure = broken sensor, not a clumsy user
```

## Considered options

- **`-1` as another gesture ID.** Rejected. It is the complement of the confident classes: a
  large, undefined region a person cannot aim at on purpose, whose boundary moves with the
  threshold and with every retrain. A PIN containing it is a secret whose reproduction rate is
  unknown. The gain over four gestures is about one bit.
- **`-1` counts as a wrong digit.** Rejected. It costs no code, but produces a `PIN_WRONG` that
  the user cannot tell apart from having misremembered the PIN, so they retry blind. It also turns
  interference into a failed attempt instead of a repeated gesture, and the usual answer to that is
  to lower `GESTURE_MODEL_THRESHOLD`, which weakens the lock.
- **Resetting by itself after a run of unsure.** Rejected in favour of rules 3 and 4 together. A
  lock that sometimes clears itself leaves the user unable to answer "how many gestures am I in?",
  so they start over to be sure. An explicit reset is predictable and reuses the double tap that is
  already the clear gesture.

## Consequences

- Every symbol in a stored PIN is one the model is confident about, so a PIN that enrols can be
  entered again, and survives a retrain or a threshold change.
- A sensor miss costs one repeated gesture instead of the whole attempt. This matters because the
  threshold is tuned for a low false-accept rate, which buys a non-trivial false-reject rate.
- Guesses for a four-gesture PIN stay at 4^4. Nobody can choose to produce `-1`, and a gesture the
  model is confident about always fills a slot, so repeating never widens an attacker's budget.
  "Unsure" is decided before any comparison, so the prompt reveals nothing about the stored PIN.
- Rule 4 is load-bearing, not a nicety: without it an abandoned entry parks a partly filled buffer,
  and the next person at the door needs only the rest of the gestures. Two correct digits left
  behind turn 4^4 guesses into 4^2.
- Rule 3 is fault detection, not a defence against guessing. It gives a defined exit from states
  that otherwise never end, such as `network_ready()` failing on every call or the IMU dropping off
  the I2C bus.
- The prompt has to say *two quick taps*. `BUTTON_TAP_MAX_MS` is 200 ms, so a deliberate press
  returns `BUTTON_GESTURE`, classifies a motionless recording as `-1` and deepens the run it was
  meant to escape. A wedged button defeats rule 3 entirely, because `button_update()` returns
  `BUTTON_NONE` while the level never changes; only rule 4 still works then.
- `STUCK` reaches only the USART2 debug console until `link.c` exists, so rule 3 needs an LED
  pattern as well to be any use at the door.
- The five-in-a-row count and the 20-30 s timeout are reasoned from the code, not measured. Re-check
  both against a real false-reject rate from `ml/`.
- Separately from this decision: `pin_add()` returning `PIN_WRONG` on the first mismatch would leak
  which digit was wrong, collapsing 4^4 guesses to about 4^2. Compare only once every digit is in.
