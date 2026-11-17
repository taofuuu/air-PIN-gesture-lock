# Air PIN Gesture Lock

A door lock that opens when someone performs the right sequence of gestures in the air with a
handheld wand.

## Language

### The wand

**Wand**:
The 3D-printed handle that holds the motion sensor and the button; the thing a person moves to
perform gestures.
_Avoid_: controller, stick, remote

**Button**:
The push button on top of the wand. It is held down for exactly the length of one gesture.
_Avoid_: trigger, key

**Grip**:
How the wand sits in the hand. The standard grip is like a laser pointer, with the button on top
under the thumb.
_Avoid_: hold, pose

**Starting pose**:
The wand held still in the standard grip, tip aimed straight ahead at chest height. Every gesture
starts from it.
_Avoid_: rest position, home

**Twist**:
Rotation of the wand around its own length, away from button-on-top.
_Avoid_: roll, grip change

**Gravity correction**:
Undoing twist by using the direction of gravity, measured while the wand is still, as the
reference for "up".
_Avoid_: straightening, levelling

### Gestures

**Gesture**:
The path the wand's tip draws during one button press, starting from the starting pose, as if
drawing on a wall with a laser pointer. Its direction, orientation and proportions are part of it;
its speed, its overall size, twisting the wand and moving the whole arm are not.
_Avoid_: move, motion, symbol, shape

**Gesture set**:
The gestures one lock has been taught. The PIN is picked from it, and it holds more gestures than
the PIN uses.
_Avoid_: vocabulary, library, dictionary

**Enrolment**:
Teaching the lock one new gesture by performing it several times, which adds it to the gesture set.
_Avoid_: training, registration, recording a gesture

**Stroke**:
A gesture that tilts the tip in one straight direction from the starting pose and stops there.
UP, DOWN, LEFT and RIGHT are the strokes.
_Avoid_: flick, swipe

**UP / DOWN / LEFT / RIGHT**:
From the starting pose, one quick wrist tilt that moves the tip about 30–45° up, down, left or
right as the performer sees it, ending still.

**Return stroke**:
Bringing the tip back to the starting pose after a stroke. It is never part of a gesture: it
happens after the button is released.

**Unsure**:
The outcome when the lock cannot tell which gesture was performed. It matches no gesture.
_Avoid_: unknown, other, none

**PIN**:
The secret, ordered sequence of gestures from the gesture set that opens the lock. Anyone who
knows it and performs it opens the lock: it says nothing about who the person is.
_Avoid_: code, password, combination

**PIN setup**:
Choosing the PIN from the gesture set.
_Avoid_: PIN enrolment, recording a PIN

### Motion data

**Sample**:
One instant of motion-sensor output: three acceleration and three rotation-rate values.
_Avoid_: reading, frame, point

**Recording**:
Everything captured for one button press: its lead-in plus the samples from press to release.
_Avoid_: take, clip, trial

**Lead-in**:
The few samples just before the button press, while the wand is still in the starting pose. Part
of a recording, but not of the gesture.
_Avoid_: pre-roll, preamble

**Session**:
One sitting in which one person makes recordings.
_Avoid_: run, batch

**Junk recording**:
A recording of a motion that is no gesture, such as a wiggle or an accidental press. It exists
only to check that the lock answers unsure.
_Avoid_: noise, negative, other

**Twisted recording**:
A recording of a real gesture made with the wand twisted. It exists only to check gravity
correction.

**Enrolment recording**:
A recording made during enrolment. It belongs to the lock's owner and is never used to train the
network.
_Avoid_: enrolment sample
