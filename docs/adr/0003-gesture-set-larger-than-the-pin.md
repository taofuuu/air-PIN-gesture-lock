---
Status: superseded by ADR 0006
---

# The lock is taught a gesture set larger than the PIN

People enrol their own gestures on the lock instead of choosing from a fixed list, which
[ADR 0001](0001-classifier-first-embedding-later.md) named as the goal after the classifier. What
they enrol is a gesture set, and PIN setup then picks the PIN from it as an ordered sequence, with
repeats allowed. The set holds more gestures than the PIN uses, so the lock recognising a motion
says nothing about whether that motion is in the PIN.

The lock checks what was performed, not who performed it: anyone who knows the PIN and performs
its gestures opens the door, the same as with a keypad.

## Considered options

- **The PIN is the gestures.** PIN setup enrols each position in turn and the lock knows nothing
  else. Rejected: every motion the lock recognises is then in the PIN, and because an unsure
  gesture only asks again ([ADR 0002](0002-unsure-gestures-are-never-pin-input.md)), an attacker
  can try motions until one fills a slot and find the PIN's gestures one at a time.
- **Also check who performs it.** Rejected: it needs many recordings of the owner, rejects the
  owner on an off day, and is a different and harder problem than reading a gesture.

## Consequences

- A lock ships with an empty gesture set. Not even the strokes are taught in advance, so an owner
  can't lean on shapes every attacker already knows; anyone may still enrol them.
- PIN setup needs at least 6 gestures in the set, and the set holds at most 16. An attacker who
  doesn't know which gestures are in the set has no list to guess from. One who does needs up to
  (size of the gesture set)^4 guesses for a four-gesture PIN, at least 6^4 = 1,296, instead of the
  4^4 in ADR 0002.
- The network must read the same gesture alike whoever performs it, so it is always tested on a
  person it never trained on.
- Forgetting a gesture that the current PIN uses is refused: set a new PIN first. Re-enrolling an
  existing gesture under the same ID is left out for now; forgetting it and enrolling it again
  does the same job. A forgotten gesture's ID is never given to a later one, so nothing left
  pointing at it can silently mean a different gesture.
