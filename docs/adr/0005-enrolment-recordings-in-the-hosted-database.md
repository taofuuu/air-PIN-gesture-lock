---
Status: accepted
---

# Enrolment recordings live in the hosted database

A model update changes how the network describes motion, so a lock has to rebuild its gestures from
the enrolment recordings they came from
([ADR 0004](0004-cosine-margin-network-over-template-matching.md)). Those recordings are kept in the
hosted database behind the dashboard, which also manages the gesture set, not in the lock's flash.

1. **Recognition never needs the network.** Each gesture's description stays in the lock's own
   flash, and the lock reads gestures offline.
2. **The lock rebuilds its gestures itself** from the recordings it downloads. The server never
   runs the model, so there is no second copy of the model to keep in step.
3. **Enrolment recordings are never training data.** They are the owner's secret.
4. **Every stored gesture is marked with the model version that described it.** On start-up, a
   mismatch makes the lock answer unsure to everything until it has downloaded the recordings and
   rebuilt, keeping the same gesture IDs and the same PIN. If the database can't be reached, the
   lock stays shut.

## Considered options

- **Keep the recordings in the lock's flash.** About 12 KB per gesture fits, and the secret never
  leaves the lock. Rejected: a full-chip erase when flashing new firmware wipes them, and the
  gesture set would be split between the lock and the dashboard that manages it.
- **Enrol everything again after each model update.** Rejected: every update would cost the owner
  every gesture and the PIN.
- **Never update the model after release.** Rejected: it rules out ever fixing the model.

## Consequences

- A leak of the database shows the shapes in every owner's gesture set, which undoes most of what
  [ADR 0003](0003-gesture-set-larger-than-the-pin.md) gains. The attacker still needs the PIN's
  order, as long as the PIN itself is never stored there. Access to the database is as sensitive
  as the lock.
- A lock that has just been updated stays shut until it reaches the database. Whether there is
  another way in meanwhile, such as a key or an unlock from the dashboard, is outside this
  decision.
