"""Run with:  python -m unittest discover ml"""
import tempfile
import unittest
from pathlib import Path

import numpy as np

from collect import recordings, save
from dataset import Recording, load, read
from train import GRAVITY_SAMPLES, MAX_SAMPLES, one_movement, preprocess


def line(btn, v=0):
    return f"{v},{v},8192,{v},{v},{v},{btn}\r\n"


class SplitStream(unittest.TestCase):
    def test_cuts_press_to_release_with_lead_in(self):
        stream = (["WHO_AM_I = 0x68\r\n"] + [line(0, v) for v in range(7)] + ["12,3\r\n"]
                  + [line(1, 10), line(1, 11), line(1, 12), line(0)]
                  + [line(0, 20), line(0, 21), line(1, 30), line(1, 31), line(0)])
        first, second = recordings(stream)
        self.assertEqual([r[0] for r in first], [2, 3, 4, 5, 6, 10, 11, 12])
        # Lead-in restarts after each release, so it never holds samples from the previous gesture
        self.assertEqual([r[0] for r in second], [20, 21, 30, 31])

    def test_button_stuck_down_stops_with_a_hint(self):
        with self.assertRaises(SystemExit):
            list(recordings(line(1) for _ in range(2000)))


class LoadData(unittest.TestCase):
    def test_round_trip_and_eval_folders(self):
        with tempfile.TemporaryDirectory() as tmp:
            data = Path(tmp)
            take = [[0, 0, 8192, 0, 0, 0, 0]] * 5 + [[1, 2, 3, 4, 5, 6, 1]] * 40
            save(take, "up", "toa", data)
            save(take, "down", "chin", data)
            save(take, "_junk", "toa", data)
            save(take, "_twisted/up", "toa", data)
            classes, recs = load(data)
        self.assertEqual(classes, ["down", "up"])
        self.assertEqual(sorted(r.gesture for r in recs), ["_junk", "_twisted/up", "down", "up"])
        up = next(r for r in recs if r.gesture == "up")
        self.assertEqual((up.person, up.lead_in.shape, up.samples.shape), ("toa", (5, 6), (40, 6)))
        np.testing.assert_array_equal(up.samples[0], [1, 2, 3, 4, 5, 6])

    def test_rejects_btn_out_of_order(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = save([[0] * 6 + [1], [0] * 6 + [0]], "up", "toa", Path(tmp))
            with self.assertRaises(ValueError):
                read(path, Path(tmp))


def rolled(rows, degrees):
    """Rows of raw counts as the sensor would read them with the wand twisted by degrees."""
    c, s = np.cos(np.radians(degrees)), np.sin(np.radians(degrees))
    out = rows.astype(float)
    for y, z in ((1, 2), (4, 5)):  # accel, then gyro
        out[:, y], out[:, z] = c * rows[:, y] + s * rows[:, z], c * rows[:, z] - s * rows[:, y]
    return out.round().astype(np.int16)


class Preprocess(unittest.TestCase):
    samples = np.random.default_rng(0).integers(-9000, 9000, (60, 6)).astype(np.int16)
    samples[:GRAVITY_SAMPLES] = [300, 0, 8192, 0, 0, 0]  # the hold starts with the wand still

    def rec(self, samples):
        return Recording("up", "toa", samples[:0], samples, Path("x.csv"))

    def test_undoes_twist_in_either_direction(self):
        straight = preprocess(self.rec(self.samples))
        for degrees in (-70, 45, 90):
            np.testing.assert_allclose(preprocess(self.rec(rolled(self.samples, degrees))), straight, atol=1e-3)

    def test_scales_and_pads_to_the_recording_buffer(self):
        x = preprocess(self.rec(self.samples))
        self.assertEqual(x.shape, (6, MAX_SAMPLES))
        np.testing.assert_allclose(x[:, 0], self.samples[0] / 32768)
        self.assertFalse(x[:, 60:].any())


class OneMovement(unittest.TestCase):
    def check(self, gz_deg_per_s, expected):
        """gz_deg_per_s holds one 0.1 s stretch of rotation per entry."""
        x = np.zeros((1, 6, MAX_SAMPLES), np.float32)
        x[0, 5, :10 * len(gz_deg_per_s)] = np.repeat(gz_deg_per_s, 10) / 500
        self.assertEqual(one_movement(x)[0], expected)

    def test_a_stroke_passes(self):
        self.check([300, 100], True)   # 40 degrees, one way

    def test_a_wiggle_fails(self):
        self.check([300, -100, 300, -200], False)  # travels 90 degrees, ends 30 from the start

    def test_a_still_press_fails(self):
        self.check([20, 20, 20], False)  # sensor drift: straight, but only 6 degrees


if __name__ == "__main__":
    unittest.main()
