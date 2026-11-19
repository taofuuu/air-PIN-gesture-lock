---
Status: accepted
---

# Each user opens the lock with one secret gesture

Each user enrols one secret gesture of their own, and performing any user's secret gesture opens
the lock. There is no PIN to pick and no set of gestures to pick it from: the gesture is the whole
secret. This replaces [ADR 0003](0003-gesture-set-larger-than-the-pin.md), where each lock learned a
gesture set larger than the PIN.

The lock still checks what was performed, not who performed it: anyone who knows a secret gesture
can open the door with it. Because each secret gesture belongs to one user, the lock knows whose
gesture opened it.

## Considered options

- **An enrolled gesture set with a PIN picked from it (ADR 0003).** Rejected. Decoys make the owner
  enrol gestures they never use, at least 30 tries. Without decoys, the lock reads each gesture on
  its own, so an attacker finds the PIN's gestures one at a time and then has at most 24 orders to
  try. A set shared by several users also shows every user's shapes to the others, and a user who
  leaves keeps knowing them.
- **A fixed pool of about 20 known shapes, with a PIN picked from them.** No enrolment and the
  strongest countable security (20^4 = 160,000 guesses), but the lock never learns anything, and the
  goal is a lock that learns gestures from its users. It would also have to read a new person's
  style from only three people's recordings.

## Consequences

- Security is no longer a count of guesses. It is the measured share of other gestures that open
  the lock, which depends on one threshold. Loosening it for convenience weakens the lock directly.
- Enrolment refuses a secret gesture too close to a common shape, such as a single stroke or a plain
  circle, because those are what an attacker tries first.
- A secret gesture is judged as a whole, so a failed try gives no partial clue. A readable gesture
  that matches no secret gesture is a failed attempt; only an unreadable recording (unsure) is asked
  again without counting. Otherwise an attacker could try gestures without limit.
- Two users' secret gestures may be close. Enrolment doesn't refuse that, because refusing would
  tell the new user that someone else's secret looks like theirs. Both still open the lock; only
  which user is credited can be wrong.
- Removing a user means deleting their secret gesture. Nothing they knew helps them afterwards.
- A secret gesture is longer than a stroke, so it is harder to repeat exactly, and a failed try costs
  the whole attempt instead of one slot.
- The four-stroke lock, with its PIN, PIN setup and the slot rules of
  [ADR 0002](0002-unsure-gestures-are-never-pin-input.md), stays as the baseline and is finished
  regardless. If secret gestures aren't ready in time or fail on the board, the four-stroke lock is
  what ships. Once secret gestures meet their targets and work end to end on the board, the
  four-stroke lock and its PIN slots are removed and only secret gestures remain.
- Enrolment recordings are now the secrets themselves: anyone who sees one knows what to perform.
  That matters wherever they are kept ([ADR 0005](0005-enrolment-recordings-in-the-hosted-database.md)).
