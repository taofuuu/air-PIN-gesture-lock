"""Train the gesture classifier on ml/data/, evaluate it and export it for gesture.c.

    python ml/train.py

Cross-validates on the gesture folders and sets the unsure threshold from that, then trains the
final model on all of them and tests it on _twisted/ and _junk/, which it never trains on. With more
than one person in the data, cross-validation holds out one person at a time.

The lock answers unsure when the model's confidence is below the threshold, or when the recording
fails one_movement(). Most junk is only caught by the second check.

Writes ml/model/gesture_model.onnx for X-CUBE-AI and gesture_model.h for gesture.c. Needs torch and
onnx, imported late so the tests run without them.
"""
from pathlib import Path

import numpy as np

from collect import MAX_SAMPLES, RATE_HZ
from dataset import load

GESTURES = ["up", "down", "left", "right"]  # the order of gesture_t in gesture.h
FOLDS = 5
EPOCHS = 150
STROKES_KEPT = 0.98  # share of correctly classified strokes the unsure threshold must let through
# shortcut: both limits are hand-set from one person's strokes, recheck them when teammates record
MIN_TURN = 15.0          # degrees; the weakest real stroke so far turned 19
MIN_STRAIGHTNESS = 0.5   # degrees turned per degree travelled; the wobbliest real stroke scored 0.51
GRAVITY_SAMPLES = 5  # gesture_classify() gets no lead-in, so gravity comes from the start of the hold
ONNX = Path(__file__).parent / "model" / "gesture_model.onnx"
HEADER = ONNX.with_suffix(".h")


def preprocess(r):
    """One recording as the model's input, (6, MAX_SAMPLES) floats. gesture.c must repeat this exactly."""
    x = r.samples.astype(np.float32) / 32768  # 1.0 = 4 g or 500 deg/s, the sensor's full scale
    _, ay, az = r.samples[:GRAVITY_SAMPLES, :3].mean(0)
    roll = np.arctan2(ay, az)  # gravity correction: turn around the wand's length (x) until gravity is z
    c, s = np.cos(roll), np.sin(roll)
    for y, z in ((1, 2), (4, 5)):  # accel, then gyro
        x[:, y], x[:, z] = c * x[:, y] - s * x[:, z], s * x[:, y] + c * x[:, z]
    return np.pad(x, ((0, MAX_SAMPLES - len(x)), (0, 0))).T


def batch(recordings):
    import torch
    return torch.from_numpy(np.stack([preprocess(r) for r in recordings]))


def build():
    from torch import nn
    return nn.Sequential(
        nn.Conv1d(6, 16, 7, stride=2, padding=3), nn.ReLU(),
        nn.Conv1d(16, 32, 5, stride=2, padding=2), nn.ReLU(),
        nn.Conv1d(32, 32, 3, stride=2, padding=1), nn.ReLU(),
        nn.AdaptiveMaxPool1d(1), nn.Flatten(), nn.Linear(32, len(GESTURES)),
    )


def one_movement(x):
    """Whether preprocessed recordings turned far enough, mostly one way. gesture.c repeats this too.

    A wiggle turns a long way but ends up near where it started, and an accidental press barely
    turns at all. The model can't be relied on for these: it never sees them in training.
    """
    gyro = x[:, 3:] * 500  # deg/s, per recording and sample
    turned = np.linalg.norm(gyro.sum(2), axis=1) / RATE_HZ  # degrees from start to end
    path = np.linalg.norm(gyro, axis=1).sum(1) / RATE_HZ    # degrees travelled on the way
    return (turned >= MIN_TURN) & (turned >= MIN_STRAIGHTNESS * path)


def fit(x, y, seed):
    import torch
    torch.manual_seed(seed)
    model = build()
    optimizer = torch.optim.Adam(model.parameters(), 2e-3, weight_decay=1e-3)
    for _ in range(EPOCHS):
        for i in torch.randperm(len(x)).split(32):
            optimizer.zero_grad()
            torch.nn.functional.cross_entropy(model(x[i]), y[i]).backward()
            optimizer.step()
    return model.eval()


def predict(model, x):
    """Each recording's predicted gesture index and its confidence."""
    import torch
    with torch.no_grad():
        confidence, gesture = model(x).softmax(1).max(1)
    return gesture.numpy(), confidence.numpy()


def classify(model, x, threshold):
    """What the lock would answer: each recording's gesture index, and whether to call it unsure."""
    gesture, confidence = predict(model, x)
    return gesture, (confidence < threshold) | ~one_movement(x.numpy())


def folds(strokes):
    """A fold label per stroke: the person if there are several, else shuffled and even per gesture."""
    if len({r.person for r in strokes}) > 1:
        return np.array([r.person for r in strokes])
    label = np.zeros(len(strokes), int)
    rng = np.random.default_rng(0)
    gestures = np.array([r.gesture for r in strokes])
    for g in GESTURES:
        where = rng.permutation(np.flatnonzero(gestures == g))
        label[where] = np.arange(len(where)) % FOLDS
    return label


def export(model, threshold):
    import torch
    ONNX.parent.mkdir(exist_ok=True)
    # softmax inside the model, so gesture.c can compare its output to the threshold directly
    torch.onnx.export(torch.nn.Sequential(model, torch.nn.Softmax(1)), torch.zeros(1, 6, MAX_SAMPLES), ONNX,
                      input_names=["imu"], output_names=["confidence"], opset_version=13)
    HEADER.write_text("\n".join([
        "/* Generated by ml/train.py. Do not edit: retrain instead. */",
        "#ifndef GESTURE_MODEL_H", "#define GESTURE_MODEL_H", "",
        f"#define GESTURE_MODEL_SAMPLES {MAX_SAMPLES}",
        f"#define GESTURE_MODEL_RATE_HZ {RATE_HZ}",
        f"#define GESTURE_MODEL_GRAVITY_SAMPLES {GRAVITY_SAMPLES}",
        f"#define GESTURE_MODEL_THRESHOLD {threshold:.6f}f  // below this, gesture_classify() returns -1",
        f"#define GESTURE_MODEL_MIN_TURN {MIN_TURN}f",
        f"#define GESTURE_MODEL_MIN_STRAIGHTNESS {MIN_STRAIGHTNESS}f",
        f"// Outputs in order: {', '.join(GESTURES)}", "", "#endif /* GESTURE_MODEL_H */", ""]))


def main():
    import torch
    _, recordings = load()
    strokes = [r for r in recordings if r.gesture in GESTURES]
    if {r.gesture for r in strokes} != set(GESTURES):
        raise SystemExit(f"ml/data/ needs recordings of all of {GESTURES}")
    x, y = batch(strokes), np.array([GESTURES.index(r.gesture) for r in strokes], np.int64)
    print(f"strokes passing the one-movement rule: {one_movement(x.numpy()).sum()}/{len(y)}")

    label = folds(strokes)
    predicted, confidence = np.zeros(len(y), int), np.zeros(len(y))
    for seed, held_out in enumerate(np.unique(label)):
        test = label == held_out
        model = fit(x[~test], torch.from_numpy(y[~test]), seed)
        predicted[test], confidence[test] = predict(model, x[test])
    right = predicted == y
    threshold = float(np.quantile(confidence[right], 1 - STROKES_KEPT))
    print(f"cross-validated: {right.sum()}/{len(y)} right, unsure threshold {threshold:.3f}")
    confusion = np.zeros((len(GESTURES),) * 2, int)
    np.add.at(confusion, (y, predicted), 1)
    print("  true \\ predicted", *GESTURES)
    for g, row in zip(GESTURES, confusion):
        print(f"  {g:<6}", *row)

    model = fit(x, torch.from_numpy(y), len(np.unique(label)))
    twisted = [r for r in recordings if r.gesture.startswith("_twisted/")]
    if twisted:
        gesture, unsure = classify(model, batch(twisted), threshold)
        wanted = np.array([GESTURES.index(r.gesture.split("/")[1]) for r in twisted])
        print(f"twisted: {(gesture == wanted).sum()}/{len(twisted)} right, {unsure.sum()} of them unsure")
    junk = [r for r in recordings if r.gesture == "_junk"]
    if junk:
        print(f"junk: {classify(model, batch(junk), threshold)[1].sum()}/{len(junk)} unsure")
    export(model, threshold)
    print(f"wrote {ONNX} and {HEADER}")


if __name__ == "__main__":
    main()
