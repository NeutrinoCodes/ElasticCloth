//////////////////////////////////////////////////////////////////
//
//      D---------C
//      |         |
//      |         |
//      |         |
//      A---------B
//
//
//      y
//      |
//      |
//      o -----x
//     /
//    /
//   z
//
//////////////////////////////////////////////////////////////////

ds = 0.05;                                                       // Setting side discretization length...
x_min = -1.0;                                                   // Setting "x_min"...
x_max = +1.0;                                                   // Setting "x_max"...
y_min = -1.0;                                                   // Setting "y_min"...
y_max = +1.0;                                                   // Setting "y_max"...

// Nodes:
A = 1;
B = 2;
C = 3;
D = 4;

// Sides:
AB = 1;
BC = 2;
CD = 3;
DA = 4;

// Curves:
ABCD_curve = 5;

// Surfaces:
ABCD_surface = 1;

// Physical groups:
perimeter = 6;
x_side = 7;
y_side = 8;
surface = 2;

Point(A) = {x_min, y_min, 0.0, ds};                             // Setting point "A"...
Point(B) = {x_max, y_min, 0.0, ds};                             // Setting point "B"...
Point(C) = {x_max, y_max, 0.0, ds};                             // Setting point "C"...
Point(D) = {x_min, y_max, 0.0, ds};                             // Setting point "D"...

Line(AB) = {A, B};                                              // Setting side "AB"...
Line(BC) = {B, C};                                              // Setting side "BC"...
Line(CD) = {C, D};                                              // Setting side "CD"...
Line(DA) = {D, A};                                              // Setting side "AD"...

Curve Loop(ABCD_curve) = {AB, BC, CD, DA};                      // Setting perimeter "ABCD"...

Plane Surface(ABCD_surface) = {ABCD_curve};                     // Setting surface "ABCD"...

Transfinite Surface {ABCD_surface};                             // Applying transfinite algorithm...
Recombine Surface {ABCD_surface};                               // Recombining triangles into quadrangles...

Physical Curve(perimeter) = {AB, BC, CD, DA};                   // Setting group: perimeter "ABCD"...
Physical Curve(x_side) = {AB};                                  // Setting group: side "AB"...
Physical Curve(y_side) = {DA};                                  // Setting group: side "AD"...
Physical Surface(surface) = {ABCD_surface};                     // Setting group: surface "ABCD"...

Mesh 3;                                                         // Setting mesh type: quadrangles...

Mesh.SaveAll = 1;                                               // Saving all mesh nodes...