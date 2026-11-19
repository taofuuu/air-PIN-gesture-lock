---
Status: accepted
---

# A cosine-margin network reads secret gestures

Secret gestures ([ADR 0006](0006-one-secret-gesture-per-user.md)) are read by a small neural network
that turns a recording into a list of numbers. It is trained as a cosine classifier with a margin
(CosFace-style), so performances of the same gesture point the same way and different gestures stay
clearly apart. Enrolment averages the lists from five performances into the user's stored
description, and reading compares a performance with every user's description. The network is
trained once, on many gestures that are nobody's secret gesture, and never learns on the lock.

## Considered options

- **Dynamic time warping (DTW) against the enrolment recordings.** Needs no training and no data
  collection, and is strong at checking one long gesture against a few recordings of it, much like
  signature checking. Not shipped: it is weaker at telling similar gestures apart and at rejecting
  junk. It stays in `ml/` as the score the network must beat on the same held-out test; if DTW wins,
  reopen this decision.
- **A plain classifier with its last layer cut off.** It trains on raw dot products while the lock
  compares directions, and nothing keeps different gestures apart, so the threshold has no gap to
  sit in.
- **Prototypical networks.** Few-shot studies show results similar to a cosine classifier, but they
  need an episode sampler, for no gain here.
- **Triplet loss.** Slow and fiddly to train, for no gain here.

## Consequences

- The network only describes unseen secret gestures well if it trained on many different long
  gestures. Recording enough real ones is slow, so training also uses long gestures made by joining
  recorded short gestures end to end. Tests only ever use real long gestures, held out by person.
- The network sees the path the wand's tip draws: the gravity-corrected gyro turned into up/down
  and left/right angles, resampled to a fixed number of points along its length and scaled to a
  fixed size, the same in both directions so a gesture's proportions are kept. Speed, size, twist
  and arm movement are removed before the network instead of being learned away, and the
  accelerometer only gives gravity correction. If the held-out test shows raw sensor values do
  better, revisit this.
- The network keeps the order of a gesture's parts: it pools into a few segments along the path
  instead of taking one maximum over all of it, as the four-stroke classifier does. Every path has
  the same number of points, so a position in the input always means the same fraction of the way
  along the gesture. One global maximum would give circle-then-Z and Z-then-circle almost the same
  description.
- Before the network runs, a recording is junk if the tip travels less than a minimum distance
  (about 15° to start, tuned on junk recordings) or if most of its rotation is twist. The old rule
  that a gesture must end away from where it started is dropped: loops end where they began.
- The lock opens when the nearest secret gesture is within one fixed threshold, tuned on the PC from
  held-out people, other gestures and junk. When two users' gestures are both near enough, the
  nearest is credited. Five enrolment performances are too few to set a threshold per user.
- Enrolment takes five performances. One that doesn't match the others is asked for again. If the
  same performance fails three times in a row, the tries so far are thrown away and the user can
  start over or cancel, because the bad try is often an earlier one. A secret gesture too close to a
  common shape is refused. Nothing is saved until five tries agree, so a cancel or a power cut
  changes nothing.
- A retrained network describes motion differently, so every stored description must be rebuilt
  after a model update ([ADR 0005](0005-enrolment-recordings-in-the-hosted-database.md)).
- The four-stroke classifier stays on `main` as the baseline until this model meets its accuracy
  targets on held-out people and the secret-gesture lock works end to end on the board. Only then
  is it replaced completely, `gesture_t` names included.
