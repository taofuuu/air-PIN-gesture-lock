// Air PIN gesture wand: a faceted handle in two halves (base + lid), a button cap, a round pommel,
// a decorative shaft, and two square pins that key the pommel and shaft onto the handle.
// The GY-521 and KY-004 only sit on printed pegs and are clamped by the lid,
// so they come back out undamaged. Sizes in mm, from measurements/README.md;
// values marked ESTIMATE are not measured.
// Axes: X runs from the back of the handle to the shaft, Y to the left, Z up.

/* [Output] */
// Each part comes out in its print orientation; "assembled" is for looking only.
part = "plate"; // [plate, base, lid, cap, pommel, shaft, pin, assembled]

/* [GY-521 (MPU-6050)] */
mpu_w = 20.6;          // header edge, lies across the wand
mpu_d = 15.9;          // other edge, lies along the wand
mpu_t = 1.5;           // board thickness
mpu_hole_d = 3.3;      // mounting hole diameter
mpu_hole_front = 2.05; // hole centre to the edge opposite the header
mpu_hole_side = 2.45;  // hole centre to the nearest side edge
mpu_strip_h = 2.3;     // black header strip under the board
mpu_parts_h = 2.5;     // ESTIMATE: tallest part on top of the board (the yellow capacitor)

/* [KY-004 button] */
// Measured flat, switch up, pins pointing at you.
btn_d = 18.1;       // pin edge to far edge
btn_w = 14.1;       // left edge to right edge
btn_t = 1.6;        // board thickness
btn_hole_d = 2.2;   // outer hole pair; the gaps say ~2.5, the smaller value keeps the pegs sure to fit
btn_hole_x = 13.05; // outer hole centres from the pin edge
btn_hole_y = 1.35;  // outer hole centres from each side edge (the pair is symmetric)
switch_w = 5.7;     // square switch body
switch_h = 4.0;     // switch body top above the board
plunger_d = 3.1;
plunger_h = 5.0;    // plunger tip above the board
switch_x = 9.7;     // switch centre from the pin edge (from the top-view photo)
switch_y = 7.2;     // switch centre from the right edge (from the top-view photo)
pin_top = 4.1;      // top of the bent pins above the board

/* [Wires] */
housing_l = 14;     // ESTIMATE: plastic housing on the end of a jumper wire
wire_bend = 5.5;    // room for a jumper wire to bend after its housing

/* [Button cap] */
cap_d = 6;          // printed cap that pokes through the lid and presses the plunger
cap_flange_d = 7.5; // flange under the lid keeps the cap from falling out; wider hits the hold-down tubes
cap_flange_t = 1;
protrude = 1.5;     // how far the cap sticks out of the lid

/* [Shaft] */
// Decorative shaft after the teammate's design: a taper with three rings.
shaft_len = 150;
shaft_root_d = 22;
shaft_tip_d = 8.7;
ring_at = [39, 80, 114]; // ring centres from the root
ring_h = 1.7;            // how far a ring stands out
ring_w = 7;
joint_d = 7;        // square pins key the pommel and shaft onto the handle
socket_depth = 15;  // pin depth in each part

/* [Decoration] */
ct = 4;             // 45-degree facets along the top edges; more would hit the GY-521's corners
cb = 6;             // 45-degree facets along the bottom edges; more would crowd the hanging jumper wires
collar_r = 12.5;    // ring where the shaft's flange meets the round shaft
collar_h = 7;       // flange plus ring at the shaft root
// Pommel as [distance behind the handle, radius]: a neck, a bulge and a domed end.
pommel_prof = [[3, 11], [6, 12.5], [10, 12.5], [14, 10.5], [18, 7], [21, 2]];

/* [Print fit] */
wall = 2.4;
clr = 0.3;          // gap around the boards
peg_clr = 0.3;      // pegs are this much thinner than the holes
joint_clr = 0.4;    // pin sockets are this much wider than the pin
lid_fit = 0.15;     // gap at the lid lip; lower it for a tighter press fit
lip_h = 2;
lip_t = 1.0;
btn_gap = 22;       // room behind the button for its jumper housings to bend down
exit_w = 10;        // wire exit slot in the floor, just behind the S-bend
exit_h = 4;
chicane_gap = 7;    // S-bend passage width

/* [Hidden] */
$fn = 48;
chicane_len = 18;
mpu_under = mpu_strip_h + housing_l + wire_bend;
W_in = max(mpu_w, btn_w) + 2 * clr;
H_in = mpu_under + mpu_t + mpu_parts_h;
W = W_in + 2 * wall;
H = H_in + 2 * wall;

mpu_x0 = wall + chicane_len;        // header edge of the GY-521
mpu_bot = wall + mpu_under;
mpu_top = mpu_bot + mpu_t;
split = mpu_top;                    // base/lid seam, so the lid clamps the GY-521 from above
lid_in = H - wall;

btn_x0 = mpu_x0 + mpu_d + btn_gap;  // pin edge of the KY-004
btn_y0 = -switch_y;                 // right edge, placed so the plunger is on the centre line
// KY-004 as high as fits under the lid: a jumper housing reaches ~0.95 above its pin, the cap flange needs 0.25 of play.
btn_top = lid_in - max(pin_top + 0.95 + 0.3, plunger_h + cap_flange_t + 0.25);
shelf_top = btn_top - btn_t;
cap_bot = btn_top + plunger_h;
px = btn_x0 + switch_x;
cav_end = btn_x0 + btn_d + 2;
L = cav_end + socket_depth + wall;
back_d = socket_depth + 1;          // solid block at the back, above the S-bend, holding the pommel's pin

mpu_pegs = [for (s = [-1, 1]) [mpu_x0 + mpu_d - mpu_hole_front, s * (mpu_w / 2 - mpu_hole_side)]];
btn_pegs = [for (y = [btn_hole_y, btn_w - btn_hole_y]) [btn_x0 + btn_hole_x, btn_y0 + y]];
mpu_ledge = W_in / 2 - (mpu_w / 2 - 0.8);  // reaches 0.8 mm under/over the GY-521's side edges
mpu_ledges_y = [-W_in / 2, W_in / 2 - mpu_ledge];
facet_in = 2 - sqrt(2);             // the cavity's facets are the outer ones less wall * facet_in

assert(btn_y0 >= -W_in / 2 && btn_y0 + btn_w <= W_in / 2, "KY-004 does not fit with its plunger on the centre line");
assert(btn_top <= split, "KY-004 would sit above the seam");
assert(cap_bot - (btn_top + switch_h) >= 0.3, "Button cap would rest on the switch body, not the plunger");
assert(min([for (p = btn_pegs) norm([p[0] - px, p[1]])]) >= (cap_flange_d + btn_hole_d + 2) / 2 + 0.3,
       "Button cap flange hits a hold-down tube: shrink cap_flange_d");
assert(btn_gap >= housing_l + 3, "btn_gap too small for the KY-004 jumper housings");
assert(wall - lip_t - lid_fit >= 0.8, "Lid wall too thin at the lip");
assert(shaft_root_d >= joint_d + joint_clr + 4, "Shaft root too thin around the pin socket");
assert((W_in - mpu_w) / 2 + (lid_in - mpu_top) >= ct - wall * facet_in, "Top facets would hit the GY-521's corners");
assert(back_d + 2 <= mpu_x0, "Pommel socket block runs into the GY-521");

// Octagonal outline of the handle, Z from 0 to H.
module body2d() let(w = W / 2) polygon([[-w + cb, 0], [w - cb, 0], [w, cb], [w, H - ct], [w - ct, H], [-w + ct, H], [-w, H - ct], [-w, cb]]);

module cavity2d(g = 0) offset(delta = g - wall) body2d();

module along_x(x0, len) translate([x0, 0, 0]) rotate([90, 0, 90]) linear_extrude(len) children();

module outer() along_x(0, L) body2d();

module cavity(g = 0) along_x(wall - g, cav_end - wall + 2 * g) cavity2d(g);

module slab(z0, h) translate([-1, -W, z0]) cube([L + 2, 2 * W, h]);

module square_socket(depth) translate([0, -(joint_d + joint_clr) / 2, -(joint_d + joint_clr) / 2]) cube([depth, joint_d + joint_clr, joint_d + joint_clr]);

// Band just outside the cavity at the seam: the base's lip, or (grown by lid_fit) the lid's recess for it.
module lip_band(t, h) intersection() {
    difference() { cavity(t); cavity(); }
    slab(split - 0.01, h + 0.01);
}

// Posts from the floor up to a board's underside, with pegs through its holes ending 0.5 above its top.
module pegged_posts(pegs, top, t, hole_d, post_d) for (p = pegs) translate([p[0], p[1], wall - 0.01]) {
    cylinder(d = post_d, h = top - wall + 0.01);
    cylinder(d = hole_d - peg_clr, h = top - wall + t + 0.5);
}

// Tubes from the lid down onto a board's top around its pegs; the peg tips fit inside.
module presser_tubes(pegs, z, hole_d) for (p = pegs) translate([p[0], p[1], z]) difference() {
    cylinder(d = hole_d + 2, h = lid_in - z + 0.01);
    translate([0, 0, -0.01]) cylinder(d = hole_d - peg_clr + 0.4, h = 1.5);
}

module shell() difference() {
    outer();
    cavity();
    translate([wall + 0.5, -exit_w / 2, -1]) cube([exit_h, exit_w, wall + 2]);
    translate([-1, 0, H / 2]) square_socket(socket_depth + 1);
    translate([L - socket_depth, 0, H / 2]) square_socket(socket_depth + 1);
}

module base() {
    intersection() { shell(); slab(-1, split + 1); }
    lip_band(lip_t, lip_h);
    intersection() {
        outer();  // keeps the parts below from poking through the bottom facets
        difference() {
            union() {
                // S-bend: the wires weave between two staggered baffles, so a tug on the cable never reaches the connectors
                translate([wall + 5, -W_in / 2, wall - 0.01]) cube([2, W_in - chicane_gap, 6]);
                translate([wall + 12, -W_in / 2 + chicane_gap, wall - 0.01]) cube([2, W_in - chicane_gap, 6]);
                // block over the S-bend for the pommel's pin; its underside bridges the cavity
                translate([wall - 0.01, -W_in / 2, wall + 6.6]) cube([back_d - wall, W_in, split - wall - 6.6]);
                // GY-521: posts with pegs through its holes, side ledges stopping short of the header strip
                pegged_posts(mpu_pegs, mpu_bot, mpu_t, mpu_hole_d, 6);
                for (y = mpu_ledges_y)
                    translate([mpu_x0 + 3.5, y, wall - 0.01]) cube([mpu_d - 3.5, mpu_ledge, mpu_under + 0.01]);
                // KY-004: rails under its side edges, starting past the header's solder joints, and small pegged posts
                // under the outer holes; the solder bumps underneath stay clear
                translate([btn_x0 + 3, -W_in / 2, wall - 0.01]) cube([btn_d - 3, btn_y0 + 1.5 + W_in / 2, shelf_top - wall + 0.01]);
                translate([btn_x0 + 3, btn_y0 + btn_w - 1.5, wall - 0.01]) cube([btn_d - 3, W_in / 2 - btn_y0 - btn_w + 1.5, shelf_top - wall + 0.01]);
                pegged_posts(btn_pegs, shelf_top, btn_t, btn_hole_d, btn_hole_d + 1);
            }
            translate([-1, 0, H / 2]) square_socket(socket_depth + 1);
        }
    }
}

module lid() difference() {
    union() {
        intersection() { shell(); slab(split, H); }
        intersection() {
            outer();
            union() {
                presser_tubes(mpu_pegs, mpu_top, mpu_hole_d);
                presser_tubes(btn_pegs, btn_top, btn_hole_d);
                // ribs over the base's side ledges sandwich the GY-521's side edges, so it can't rock about its pegs
                for (y = mpu_ledges_y)
                    translate([mpu_x0 + 3.5, y, mpu_top]) cube([mpu_d - 3.5, mpu_ledge, lid_in - mpu_top + 0.01]);
            }
        }
    }
    lip_band(lip_t + lid_fit, lip_h + lid_fit);
    translate([px, 0, lid_in - 1]) cylinder(d = cap_d + 0.4, h = wall + 2);
}

// Flipped so its top lies on the bed.
module lid_print() translate([0, 0, H]) rotate([180, 0, 0]) lid();

// Flat underside rests on the plunger, so it still works if the switch sits a little off the estimate.
// Drop it into the lid hole before closing.
module cap() {
    cap_h = H + protrude - cap_bot;
    cylinder(d = cap_flange_d, h = cap_flange_t);
    cylinder(d = cap_d, h = cap_h - cap_d / 4);
    translate([0, 0, cap_h - cap_d / 4]) scale([1, 1, 0.5]) sphere(d = cap_d);  // domed top, like a set stone
}

// The handle's outline end-on, centred on the wand's axis, for the parts that butt onto its ends.
// a = 90 for a part turned to face the front, -90 for one facing the back, so its facets line up.
module end2d(a) rotate(a) translate([0, -H / 2]) body2d();

function ring(z) = max([for (c = ring_at) abs(z - c) < ring_w / 2 ? ring_h * (1 + cos(360 * (z - c) / ring_w)) / 2 : 0]);
function shaft_r(z) = (shaft_root_d + (shaft_tip_d - shaft_root_d) * z / shaft_len) / 2 + ring(z);

// Stands on its flange: the handle's outline narrows into a ring, then the round shaft.
module shaft() difference() {
    union() {
        hull() { linear_extrude(0.01) end2d(90); translate([0, 0, collar_h - 2]) cylinder(r = collar_r, h = 0.01, $fn = 64); }
        cylinder(r = collar_r, h = collar_h, $fn = 64);
        rotate_extrude($fn = 64)
            polygon(concat([[0, 0]], [for (z = [0 : 0.5 : shaft_len]) [shaft_r(z), z]], [[0, shaft_len]]));
    }
    translate([0, 0, socket_depth / 2 - 0.01]) cube([joint_d + joint_clr, joint_d + joint_clr, socket_depth], center = true);
}

// Stands on the face that meets the handle's back: the outline narrows into a neck, bulges and ends in a dome.
module pommel() difference() {
    union() {
        hull() { linear_extrude(0.01) end2d(-90); translate([0, 0, pommel_prof[0][0]]) cylinder(r = pommel_prof[0][1], h = 0.01, $fn = 64); }
        rotate_extrude($fn = 64)
            polygon(concat([[0, pommel_prof[0][0]]], [for (p = pommel_prof) [p[1], p[0]]], [[0, pommel_prof[len(pommel_prof) - 1][0]]]));
    }
    translate([0, 0, socket_depth / 2 - 0.01]) cube([joint_d + joint_clr, joint_d + joint_clr, socket_depth], center = true);
}

module pin() translate([-joint_d / 2, -joint_d / 2, 0]) cube([joint_d, joint_d, 2 * socket_depth - 1]);

// Rough stand-ins for the boards and jumper housings, to check the fit by eye.
module ghosts() {
    translate([mpu_x0, -mpu_w / 2, mpu_bot]) cube([mpu_d, mpu_w, mpu_t]);
    translate([mpu_x0, -mpu_w / 2, mpu_bot - mpu_strip_h - housing_l]) cube([2.6, 4 * 2.54, mpu_strip_h + housing_l]);
    translate([btn_x0, btn_y0, shelf_top]) cube([btn_d, btn_w, btn_t]);
    translate([px - switch_w / 2, -switch_w / 2, btn_top]) cube([switch_w, switch_w, switch_h]);
    translate([px, 0, btn_top]) cylinder(d = plunger_d, h = plunger_h);
    translate([btn_x0 + 2 - housing_l, btn_y0 + btn_w / 2 - 3.8, btn_top + pin_top - 1.6]) cube([housing_l, 3 * 2.54, 2.54]);
}

if (part == "plate") {  // everything but the tall shaft, laid out for one print
    base();
    translate([0, W + 10, 0]) lid_print();
    translate([L + 20, 0, 0]) pommel();
    translate([L + 20, W + 15, 0]) cap();
    translate([L + 45, -5, 0]) pin();
    translate([L + 45, 10, 0]) pin();
} else if (part == "base") {
    base();
} else if (part == "lid") {
    lid_print();
} else if (part == "cap") {
    cap();
} else if (part == "pommel") {
    pommel();
} else if (part == "shaft") {
    shaft();
} else if (part == "pin") {
    pin();
} else {
    base();
    %lid();
    translate([px, 0, cap_bot]) cap();
    translate([0, 0, H / 2]) rotate([0, -90, 0]) pommel();
    translate([L, 0, H / 2]) rotate([0, 90, 0]) shaft();
    color("red") ghosts();
}
