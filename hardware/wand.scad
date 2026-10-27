// Air PIN gesture wand: 3D-printed handle in two halves (base + lid).
// The GY-521 and KY-004 only sit on printed pegs and are clamped by the lid,
// so they come back out undamaged. Sizes in mm; set every MEASURE value from
// your own boards with calipers before printing.
// Axes: X runs from the back of the handle to the tip, Y to the left, Z up.

/* [Output] */
part = "print"; // [print, assembled, base, lid]

/* [GY-521 (MPU-6050)] */
mpu_w = 21.2;         // MEASURE: header edge, lies across the wand
mpu_d = 16.4;         // MEASURE: other edge, lies along the wand
mpu_t = 1.6;          // MEASURE: board thickness
mpu_hole_d = 3.0;     // MEASURE: mounting hole diameter
mpu_hole_front = 2.5; // MEASURE: hole centre to the edge opposite the header
mpu_hole_side = 2.5;  // MEASURE: hole centre to the nearest side edge
mpu_under = 22;       // header strip 2.5 + jumper housing 14 + room to bend the wire
mpu_parts_h = 2.5;    // tallest part on top of the board (the yellow capacitor)

/* [KY-004 button] */
// Measure with the board flat, switch up, pins pointing at you.
btn_d = 18.5;      // MEASURE: pin edge to far edge
btn_w = 15.0;      // MEASURE: left edge to right edge
btn_t = 1.6;       // MEASURE: board thickness
btn_hole_d = 3.0;  // MEASURE: mounting hole diameter
btn_hole_x = 15.5; // MEASURE: hole centre from the pin edge
btn_hole_y = 12.5; // MEASURE: hole centre from the right edge
plunger_x = 10;    // MEASURE: plunger centre from the pin edge
plunger_y = 7.5;   // MEASURE: plunger centre from the right edge
plunger_d = 3.5;   // MEASURE: plunger diameter
plunger_h = 8.5;   // MEASURE: plunger tip height above the board
switch_h = 3.5;    // MEASURE: switch body height above the board

/* [Shaft] */
shaft_d = 7.0;     // MEASURE: chopstick or pencil diameter
socket_depth = 15;

/* [Print fit] */
wall = 2.4;
clr = 0.3;         // gap around the boards
peg_clr = 0.3;     // pegs are this much thinner than the holes
lid_fit = 0.15;    // gap at the lid lip; lower it for a tighter press fit
lip_h = 2;
lip_t = 1.0;
r_out = 4;         // outer edge rounding
protrude = 1.5;    // how far the plunger sticks out of the lid
btn_gap = 22;      // room behind the button for its jumper housings to bend down
exit_w = 10;       // wire exit in the back wall
exit_h = 4;
chicane_gap = 7;   // S-bend passage width

/* [Hidden] */
$fn = 48;
chicane_len = 18;
W_in = max(mpu_w, btn_w) + 2 * clr;
H_in = mpu_under + mpu_t + mpu_parts_h;
W = W_in + 2 * wall;
H = H_in + 2 * wall;
r_in = max(r_out - wall, 0.5);

mpu_x0 = wall + chicane_len;        // header edge of the GY-521
mpu_bot = wall + mpu_under;
mpu_top = mpu_bot + mpu_t;
split = mpu_top;                    // base/lid seam, so the lid clamps the GY-521 from above
lid_in = H - wall;

btn_x0 = mpu_x0 + mpu_d + btn_gap;  // pin edge of the KY-004
btn_y0 = -plunger_y;                // right edge, placed so the plunger is on the centre line
shelf_top = H + protrude - btn_t - plunger_h;
switch_top = shelf_top + btn_t + switch_h;
px = btn_x0 + plunger_x;
cav_end = btn_x0 + btn_d + 2;
L = cav_end + socket_depth + wall;

mpu_pegs = [for (s = [-1, 1]) [mpu_x0 + mpu_d - mpu_hole_front, s * (mpu_w / 2 - mpu_hole_side)]];
mpu_ledge = W_in / 2 - (mpu_w / 2 - 0.8);  // reaches 0.8 mm under/over the GY-521's side edges
mpu_ledges_y = [-W_in / 2, W_in / 2 - mpu_ledge];
btn_peg = [btn_x0 + btn_hole_x, btn_y0 + btn_hole_y];

assert(btn_y0 >= -W_in / 2 && btn_y0 + btn_w <= W_in / 2, "KY-004 does not fit with its plunger on the centre line");
assert(shelf_top + btn_t <= split, "KY-004 would sit above the seam: lower protrude");
assert(switch_top <= lid_in, "Switch body hits the lid: lower protrude");
assert(btn_gap >= 17, "btn_gap too small for the KY-004 jumper housings");
assert(wall - lip_t - lid_fit >= 0.8, "Lid wall too thin at the lip");

module rrect(w, h, r) offset(r = r) square([w - 2 * r, h - 2 * r], center = true);

// Rounded bar along +X from x0, centred on the handle's axis.
module bar(x0, len, w, h, r)
    translate([x0, 0, H / 2]) rotate([90, 0, 90]) linear_extrude(len) rrect(w, h, r);

module cavity(g = 0) bar(wall - g, cav_end - wall + 2 * g, W_in + 2 * g, H_in + 2 * g, r_in + g);

module slab(z0, h) translate([-1, -W, z0]) cube([L + 2, 2 * W, h]);

// Band just outside the cavity at the seam: the base's lip, or (grown by lid_fit) the lid's recess for it.
module lip_band(t, h) intersection() {
    difference() { cavity(t); cavity(); }
    slab(split - 0.01, h + 0.01);
}

module shell() difference() {
    bar(0, L, W, H, r_out);
    cavity();
    translate([-1, -exit_w / 2, wall]) cube([wall + 2, exit_w, exit_h]);
    translate([L - socket_depth, 0, H / 2]) rotate([0, 90, 0]) cylinder(d = shaft_d + 0.3, h = socket_depth + 1);
}

module base() {
    intersection() { shell(); slab(-1, split + 1); }
    lip_band(lip_t, lip_h);
    // S-bend: the wires weave between two staggered baffles, so a tug on the cable never reaches the connectors
    translate([wall + 5, -W_in / 2, wall - 0.01]) cube([2, W_in - chicane_gap, 6]);
    translate([wall + 12, -W_in / 2 + chicane_gap, wall - 0.01]) cube([2, W_in - chicane_gap, 6]);
    // GY-521: posts with pegs through its holes, side ledges stopping short of the header strip
    for (p = mpu_pegs) translate([p[0], p[1], wall - 0.01]) {
        cylinder(d = 6, h = mpu_under + 0.01);
        cylinder(d = mpu_hole_d - peg_clr, h = mpu_under + mpu_t + 0.8);
    }
    for (y = mpu_ledges_y)
        translate([mpu_x0 + 3.5, y, wall - 0.01]) cube([mpu_d - 3.5, mpu_ledge, mpu_under + 0.01]);
    // KY-004: rails under its side edges and a small pegged boss, leaving the solder joints underneath clear
    translate([btn_x0, -W_in / 2, wall - 0.01]) cube([btn_d, btn_y0 + 1.5 + W_in / 2, shelf_top - wall + 0.01]);
    translate([btn_x0, btn_y0 + btn_w - 1.5, wall - 0.01]) cube([btn_d, W_in / 2 - btn_y0 - btn_w + 1.5, shelf_top - wall + 0.01]);
    translate([btn_peg[0], btn_peg[1], wall - 0.01]) {
        cylinder(d = btn_hole_d + 2, h = shelf_top - wall + 0.01);
        cylinder(d = btn_hole_d - peg_clr, h = shelf_top - wall + btn_t + 0.5);
    }
}

module lid() difference() {
    union() {
        intersection() { shell(); slab(split, H); }
        // tubes press the GY-521 down around its holes; the peg tips fit inside
        for (p = mpu_pegs) translate([p[0], p[1], mpu_top]) difference() {
            cylinder(d = mpu_hole_d + 2.6, h = lid_in - mpu_top + 0.01);
            translate([0, 0, -0.01]) cylinder(d = mpu_hole_d + 0.6, h = 1.5);
        }
        // ribs over the base's side ledges sandwich the GY-521's side edges, so it can't rock about its pegs
        for (y = mpu_ledges_y)
            translate([mpu_x0 + 3.5, y, mpu_top]) cube([mpu_d - 3.5, mpu_ledge, lid_in - mpu_top + 0.01]);
        // collar presses the switch body down so the KY-004 can't lift off its peg
        translate([px, 0, switch_top]) cylinder(d = 5.5, h = lid_in - switch_top + 0.01);
    }
    lip_band(lip_t + lid_fit, lip_h + lid_fit);
    translate([px, 0, switch_top - 1]) cylinder(d = plunger_d + 0.8, h = H);
}

// Rough stand-ins for the boards and jumper housings, to check the fit by eye.
module ghosts() {
    translate([mpu_x0, -mpu_w / 2, mpu_bot]) cube([mpu_d, mpu_w, mpu_t]);
    translate([mpu_x0, -mpu_w / 2, mpu_bot - 16.5]) cube([2.6, 4 * 2.54, 16.5]);
    translate([btn_x0, btn_y0, shelf_top]) cube([btn_d, btn_w, btn_t]);
    translate([px - 3, -3, shelf_top + btn_t]) cube([6, 6, switch_h]);
    translate([px, 0, shelf_top + btn_t]) cylinder(d = plunger_d, h = plunger_h);
    translate([btn_x0 - 16.5, btn_y0 + btn_w / 2 - 3.8, shelf_top + btn_t]) cube([16.5, 3 * 2.54, 2.54]);
}

if (part == "print") {
    base();
    translate([0, W + 10, H]) rotate([180, 0, 0]) lid();  // lid flipped: its top lies on the bed
} else if (part == "assembled") {  // for looking only, don't export this one
    base();
    %lid();
    color("red") ghosts();
} else if (part == "base") {
    base();
} else {
    lid();
}
