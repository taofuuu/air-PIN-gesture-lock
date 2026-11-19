---
Status: accepted
---

# Enrolment recordings live in the hosted database

A model update changes how the network describes motion, so a lock has to rebuild its secret
gestures from the enrolment recordings they came from
([ADR 0004](0004-cosine-margin-network-over-template-matching.md)). Those recordings are kept in the
hosted database behind the dashboard, which also manages the users, not in the lock's flash.

1. **Reading gestures never needs the network.** Each secret gesture's description stays in the
   lock's own flash, and the lock reads gestures offline.
2. **The lock rebuilds its secret gestures itself** from the recordings it downloads. The server
   never runs the model, so there is no second copy of the model to keep in step.
3. **Enrolment recordings are never training data.** They are their users' secrets.
4. **Every stored secret gesture is marked with the model version that described it.** On start-up,
   a mismatch makes the lock open for nobody until it has downloaded the recordings and rebuilt,
   keeping the same users. If the database can't be reached, the lock stays shut.

## Considered options

- **Keep the recordings in the lock's flash.** About 12 KB per user fits, and the secrets never
  leave the lock. Rejected: a full-chip erase when flashing new firmware wipes them, and the users
  would be split between the lock and the dashboard that manages them.
- **Enrol again after each model update.** Rejected when this was decided, when it meant every
  gesture of a gesture set plus the PIN. With one secret gesture per user
  ([ADR 0006](0006-one-secret-gesture-per-user.md)) it costs each user five tries, which reopens
  this option.
- **Never update the model after release.** Rejected: it rules out ever fixing the model.

## Consequences

- A leak of the database is a leak of every user's secret gesture: each recording shows exactly
  what to perform to open the lock. Access to the database is as sensitive as the lock itself.
- A lock that has just been updated stays shut until it reaches the database. Whether there is
  another way in meanwhile, such as a key or an unlock from the dashboard, is outside this
  decision.
