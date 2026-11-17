---
Status: accepted
---

# A cosine-margin network reads enrolled gestures

Enrolled gestures ([ADR 0003](0003-gesture-set-larger-than-the-pin.md)) are read by a small neural
network that turns a recording into a list of numbers. It is trained as a cosine classifier with a
margin (CosFace-style), so performances of the same gesture point the same way and different
gestures stay clearly apart. Enrolment averages the lists from five performances into the
gesture's stored description, and recognition picks the nearest description. The network is
trained once, on about 20 gesture types that need not be in any lock's gesture set, and never
learns on the lock.

## Considered options

- **Dynamic time warping (DTW) against the enrolment recordings.** Needs no training and no data
  collection, and is strong when the same person enrols and performs. Not shipped: it is weaker at
  telling similar gestures apart and at rejecting junk. It stays in `ml/` as the score the network
  must beat on the same held-out test; if DTW wins, reopen this decision.
- **A plain classifier with its last layer cut off.** It trains on raw dot products while the lock
  compares directions, and nothing keeps different gestures apart, so the unsure threshold has no
  gap to sit in.
- **Prototypical networks.** Few-shot studies show results similar to a cosine classifier, but they
  need an episode sampler, and with about 20 gesture types the episodes barely vary.
- **Triplet loss.** Slow and fiddly to train, for no gain here.

## Consequences

- The network only reads unseen gestures well if it trained on many gesture types, so the data
  is about 20 types from all three people, and tests always hold out whole gesture types and one
  person.
- The network sees the path the wand's tip draws: the gravity-corrected gyro turned into up/down
  and left/right angles, resampled to a fixed number of points along its length and scaled to a
  fixed size, the same in both directions so a gesture's proportions are kept. Speed, size, twist and arm movement are removed before the network instead of being
  learned away, and the accelerometer only gives gravity correction. If the held-out test shows
  raw sensor values do better, revisit this.
- Before the network runs, a recording is junk if the tip travels less than a minimum distance
  (about 15° to start, tuned on junk recordings) or if most of its rotation is twist. The old rule
  that a gesture must end away from where it started is dropped: circles end where they began.
- The lock answers unsure when the nearest gesture is further than one fixed threshold, tuned on
  the PC from held-out people and junk, or when the two nearest gestures are about equally near.
  Five enrolment performances are too few to set a threshold per gesture.
- Enrolment takes five performances. One that doesn't match the others is asked for again, and a
  gesture too close to one already in the set is refused. If the same performance fails three
  times in a row, the tries so far are thrown away and the owner can start over or cancel, because
  the bad try is often an earlier one. Nothing is saved until five tries agree, so a cancel or a
  power cut changes nothing.
- A retrained network describes motion differently, so every stored description must be rebuilt
  after a model update, from the enrolment recordings
  ([ADR 0005](0005-enrolment-recordings-in-the-hosted-database.md)).
- The four-stroke classifier stays on `main` until this model meets its accuracy targets on
  held-out gestures and people, then is replaced completely, `gesture_t` names included. There is
  never a mix of fixed and enrolled gestures.
