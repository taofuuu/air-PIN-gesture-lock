---
Status: proposed (needs the PIN-mapping and ESP32 owners to agree)
---

# Classifier first, embedding + templates later

We train a fixed gesture classifier first: it is the quickest way to prove the whole chain
(wand → serial → model → PIN) and the most accurate on gestures it was trained on. The goal after
that is on-device enrolment (embedding + templates), where a person adds their own gestures
without retraining, which makes a PIN much harder to guess. So that the switch only touches
`gesture.c` and `ml/`, every part follows two rules from now on:

1. **A gesture ID is a plain int.** PIN mapping and the dashboard store and compare it, and never
   assume how many gestures exist or which ID is which. The `GESTURE_*` names are for logs and
   tests only.
2. **`-1` means unsure**, and every caller handles it. The classifier returns it below a
   confidence threshold; the embedding model will return it when no template is close enough.

## Considered options

- **Embedding + templates from the start.** Rejected for now: it needs recordings of many
  different gestures (10+) before it learns anything useful, and it is less accurate with only a
  few enrolment recordings. The recordings we collect for the classifier serve it later unchanged.

## Consequences

- Recordings, the `ml/data/` layout, `collect.py` and the loader carry over as they are.
- Switching rewrites `train.py` and the inside of `gesture.c`, and adds an enrol mode, template
  storage in flash and a trigger to start enrolment (a long press, or a new `link.h` command).
