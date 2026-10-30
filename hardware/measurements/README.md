# Module measurements

Source for the board sizes in [`../wand.scad`](../wand.scad), taken with a digital caliper.
Each photo is named after what it measures and the reading. A **gap** runs from a hole's edge
(not its centre) to a board edge.

## GY-521 (MPU-6050)

| Photo | What it measures | mm |
|-------|------------------|----|
| `gy521_length-header-edge_20.6mm.jpg` | Board length along the header edge | 20.6 |
| `gy521_width_15.9mm.jpg` | Board width, header edge to the edge with the holes | 15.9 |
| `gy521_thickness_1.5mm.jpg` | Board thickness | 1.5 |
| `gy521_hole-diameter_3.3mm.jpg` | Mounting hole diameter | 3.3 |
| `gy521_hole-gap-front-edge_0.4mm.jpg` | Gap, hole to the edge opposite the header | 0.4 |
| `gy521_hole-gap-side-edge_0.8mm.jpg` | Gap, hole to the short side edge | 0.8 |
| `gy521_holes-inner-edge-spacing_12.4mm.jpg` | Between the inner edges of the two holes | 12.4 |
| `gy521_board-top-to-header-strip-bottom_3.8mm.jpg` | Board top to the bottom of the black header strip | 3.8 |
| `gy521_board-top-to-pin-tips_11.1mm.jpg` | Board top to the pin tips | 11.1 |

Derived: hole centres sit 2.05 from the edge opposite the header and 2.45 from each side edge.
Check: 20.6 − 2 × 2.45 = 15.7 = 12.4 + 3.3. The header strip is 3.8 − 1.5 = 2.3 tall and the
pins stick out 11.1 − 3.8 = 7.3 below it.

## KY-004 button

Measured flat, switch up, pins pointing at you. Pegs go through the outer pair of holes; the
inner pair is too close to the switch.

| Photo | What it measures | mm |
|-------|------------------|----|
| `ky004_length_18.1mm.jpg` | Board length, pin edge to far edge | 18.1 |
| `ky004_width_14.1mm.jpg` | Board width | 14.1 |
| `ky004_thickness_1.6mm.jpg` | Board thickness | 1.6 |
| `ky004_outer-hole-diameter_2.2mm.jpg` | Outer hole diameter | 2.2 |
| `ky004_outer-hole-gap-pin-edge_11.7mm.jpg` | Gap, outer hole to the pin edge | 11.7 |
| `ky004_outer-hole-gap-far-edge_3.7mm.jpg` | Gap, outer hole to the far edge | 3.7 |
| `ky004_outer-hole-gap-near-side_0.1mm.jpg` | Gap, outer hole to the near side edge | 0.1 |
| `ky004_outer-hole-gap-opposite-side_11.5mm.jpg` | Gap, outer hole to the opposite side edge | 11.5 |
| `ky004_outer-holes-inner-edge-spacing_8.9mm.jpg` | Between the inner edges of the two outer holes | 8.9 |
| `ky004_underside-to-switch-top_5.6mm.jpg` | Board underside to the top of the switch body | 5.6 |
| `ky004_underside-to-plunger-tip_6.6mm.jpg` | Board underside to the plunger tip | 6.6 |
| `ky004_solder-bumps-to-plunger-tip_7.8mm.jpg` | Solder bumps under the board to the plunger tip | 7.8 |
| `ky004_underside-to-bent-pin-top_5.7mm.jpg` | Board underside to the top of the bent pins | 5.7 |
| `ky004_switch-body-side-a_5.7mm.jpg`, `ky004_switch-body-side-b_5.7mm.jpg` | Switch body, both sides (square) | 5.7 |
| `ky004_plunger-diameter_3.1mm.jpg` | Plunger diameter | 3.1 |
| `ky004_pin-bend-to-tip_7.5mm.jpg` | Bend of a pin to its tip | 7.5 |

Derived:
- Hole centres are the midpoints between opposite gaps, so caliper error on the hole edge
  cancels: 13.05 from the pin edge (between 11.7 and 18.1 − 3.7) and 1.35 from each side edge
  (between 0.1 and 14.1 − 11.5).
- The opposite gaps imply a hole of about 2.5, and 8.9 + 2.5 = 11.4 = 14.1 − 2 × 1.35, so the
  pair is symmetric. The 2.2 reading most likely caught a chord rather than the full diameter;
  the model keeps 2.2 for the pegs so they are sure to fit.
- Switch body top is 5.6 − 1.6 = 4.0 above the board top, the plunger tip 6.6 − 1.6 = 5.0, the
  bent pins 5.7 − 1.6 = 4.1, and the solder bumps stick out 7.8 − 6.6 = 1.2 below the board.

## Switch position, from a photo

The caliper can't reach the switch body without tilting on the board, so its position comes
from `modules_top-view.webp`: a perspective fit on the board's four corners (18.1 × 14.1) maps
each pixel onto the board. The fit puts the measured outer hole at 13.01 / 1.36 (caliper: 13.05 /
1.35) and the switch legs 4.4 and 6.6 apart (a 6 × 6 switch: 4.5 and 6.5), so it is good to a few
tenths of a millimetre. Switch centre: **9.7 from the pin edge, 7.2 from the right edge**. The
button cap has a flat bottom, so it still meets the plunger if this is off by up to ~2.

## Still estimated in the model

- GY-521 tallest part on top of the board (`mpu_parts_h`, 2.5 is on the safe side).
- Jumper wire plastic housing length (`housing_l`, 14 is the common size).
