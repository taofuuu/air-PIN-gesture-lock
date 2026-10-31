"""Run with:  python -m unittest discover ml"""
import tempfile
import unittest
from pathlib import Path

import numpy as np

from collect import recordings, save
from dataset import load, read


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


if __name__ == "__main__":
    unittest.main()
