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

ds = 0.02;                                                      // Setting side discretization length...
x_min = -1.0;                                                   // Setting "x_min"...
x_max = +1.0;                                                   // Setting "x_max"...
y_min = -1.0;                                                   // Setting "y_min"...
y_max = +1.0;                                                   // Setting "y_max"...

Point(10) = {x_min, y_min, 0.0, ds};                             // Setting point "A"...
Point(20) = {x_max, y_min, 0.0, ds};                             // Setting point "B"...
Point(30) = {x_max, y_max, 0.0, ds};                             // Setting point "C"...
Point(40) = {x_min, y_max, 0.0, ds};                             // Setting point "D"...

Line(100) = {10, 20};                                               // Setting side "AB"...
Line(200) = {20, 30};                                               // Setting side "BC"...
Line(300) = {30, 40};                                               // Setting side "CD"...
Line(400) = {40, 10};                                               // Setting side "AD"...

Curve Loop(1) = {100, 200, 300, 400};                                   // Setting perimeter "ABCD"...

Plane Surface(10) = {1};                                         // Setting surface "ABCD"...

Physical Curve(1) = {100, 200, 300, 400};                               // Setting group: perimeter "ABCD"...
Physical Curve(2) = {100};                                        // Setting group: side "AB"...
Physical Curve(3) = {400};                                        // Setting group: side "AD"...

Mesh 2;                                                         // Setting mesh type: triangles...

Mesh.SaveAll = 1;                                               // Saving all mesh nodes...