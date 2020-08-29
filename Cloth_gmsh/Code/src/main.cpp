/// @file

// OPENGL:
#define INTEROP       true                                                                          // "true" = use OpenGL-OpenCL interoperability.
#define GUI_SIZE_X    800                                                                           // Window x-size [px].
#define GUI_SIZE_Y    600                                                                           // Window y-size [px].
#define GUI_NAME      "Neutrino - Cloth_gmsh"                                                       // Window name.

#ifdef __linux__
  #define SHADER_HOME "../Cloth_gmsh/Code/shader"                                                   // Linux OpenGL shaders directory.
  #define KERNEL_HOME "../Cloth_gmsh/Code/kernel"                                                   // Linux OpenCL kernels directory.
  #define GMSH_HOME   "../Cloth_gmsh/Code/mesh/"                                                    // Linux GMSH mesh directory.
#endif

#ifdef __APPLE__
  #define SHADER_HOME "../Cloth_gmsh/Code/shader"                                                   // Mac OpenGL shaders directory.
  #define KERNEL_HOME "../Cloth_gmsh/Code/kernel"                                                   // Mac OpenCL kernels directory.
  #define GMSH_HOME   "../Cloth_gmsh/Code/mesh/"                                                    // Linux GMSH mesh directory.
#endif

#ifdef WIN32
  #define SHADER_HOME "..\\..\\Cloth_gmsh\\Code\\shader"                                            // Windows OpenGL shaders directory.
  #define KERNEL_HOME "..\\..\\Cloth_gmsh\\Code\\kernel"                                            // Windows OpenCL kernels directory.
  #define GMSH_HOME   "..\\..\\Cloth_gmsh\\Code\\mesh\\"                                            // Linux GMSH mesh directory.
#endif

#define SHADER_VERT   "voxel_vertex.vert"                                                           // OpenGL vertex shader.
#define SHADER_GEOM   "voxel_geometry.geom"                                                         // OpenGL geometry shader.
#define SHADER_FRAG   "voxel_fragment.frag"                                                         // OpenGL fragment shader.
#define GMSH_MESH     "Square.msh"                                                                  // GMSH mesh.

// OPENCL:
#define QUEUE_NUM     1                                                                             // # of OpenCL queues [#].
#define KERNEL_NUM    2                                                                             // # of OpenCL kernel [#].

// INCLUDES:
#include "nu.hpp"                                                                                   // Neutrino's header file.

int main ()
{
  // KERNEL FILES:
  std::string              kernel_home;                                                             // Kernel home directory.
  std::vector<std::string> kernel_1;                                                                // Kernel_1 source files.
  std::vector<std::string> kernel_2;                                                                // Kernel_2 source files.

  // INDEXES:
  size_t                   i;                                                                       // Index [#].
  size_t                   j;                                                                       // Index [#].
  size_t                   j_min;                                                                   // Index [#].
  size_t                   j_max;                                                                   // Index [#].
  size_t                   k;                                                                       // Index [#].

  // GUI PARAMETERS (orbit):
  float                    orbit_x_init       = 0.0f;                                               // x-axis orbit initial rotation.
  float                    orbit_y_init       = 0.0f;                                               // y-axis orbit initial rotation.

  // GUI PARAMETERS (pan):
  float                    pan_x_init         = 0.0f;                                               // x-axis pan initial translation.
  float                    pan_y_init         = 0.0f;                                               // y-axis pan initial translation.
  float                    pan_z_init         = -2.0f;                                              // z-axis pan initial translation.

  // GUI PARAMETERS (mouse):
  float                    mouse_orbit_rate   = 1.0;                                                // Orbit rotation rate [rev/s].
  float                    mouse_pan_rate     = 5.0;                                                // Pan translation rate [m/s].
  float                    mouse_decaytime    = 1.25;                                               // Pan LP filter decay time [s].

  // GUI PARAMETERS (gamepad):
  float                    gamepad_orbit_rate = 1.0;                                                // Orbit angular rate coefficient [rev/s].
  float                    gamepad_pan_rate   = 1.0;                                                // Pan translation rate [m/s].
  float                    gamepad_decaytime  = 1.25;                                               // Low pass filter decay time [s].
  float                    gamepad_deadzone   = 0.1;                                                // Gamepad joystick deadzone [0...1].

  // NEUTRINO:
  neutrino*                bas                = new neutrino ();                                    // Neutrino baseline.
  opengl*                  gui                = new opengl ();                                      // OpenGL context.
  opencl*                  ctx                = new opencl ();                                      // OpenCL context.
  shader*                  S                  = new shader ();                                      // OpenGL shader program.
  queue*                   Q                  = new queue ();                                       // OpenCL queue.
  kernel*                  K1                 = new kernel ();                                      // OpenCL kernel array.
  kernel*                  K2                 = new kernel ();                                      // OpenCL kernel array.
  size_t                   kernel_sx;                                                               // Kernel dimension "x" [#].
  size_t                   kernel_sy;                                                               // Kernel dimension "y" [#].
  size_t                   kernel_sz;                                                               // Kernel dimension "z" [#].

  // NODE COLOR:
  float4G*                 color              = new float4G ();                                     // Color [].

  // NODE KINEMATICS:
  float4G*                 position           = new float4G ();                                     // Position [m].
  float4*                  velocity           = new float4 ();                                      // Velocity [m/s].
  float4*                  acceleration       = new float4 ();                                      // Acceleration [m/s^2].

  // NODE KINEMATICS (INTERMEDIATE):
  float4*                  position_int       = new float4 ();                                      // Position (intermediate) [m].
  float4*                  velocity_int       = new float4 ();                                      // Velocity (intermediate) [m/s].
  float4*                  acceleration_int   = new float4 ();                                      // Acceleration (intermediate) [m/s^2].

  // NODE DYNAMICS:
  float4*                  gravity            = new float4 ();                                      // Gravity [m/s^2].
  float1*                  stiffness          = new float1 ();                                      // Stiffness.
  float1*                  resting            = new float1 ();                                      // Resting.
  float1*                  friction           = new float1 ();                                      // Friction.
  float1*                  mass               = new float1 ();                                      // Mass [kg].

  // MESH:
  mesh*                    object             = new mesh ();                                        // Mesh object.
  size_t                   nodes;                                                                   // Number of nodes.
  size_t                   elements;                                                                // Number of elements.
  size_t                   neighbours;                                                              // Number of neighbours.
  size_t                   border_nodes;                                                            // Number of border nodes.
  std::vector<size_t>      neighbourhood;                                                           // Neighbourhood.
  std::vector<size_t>      neighbour;                                                               // Neighbour tuple.
  std::vector<size_t>      border;                                                                  // Nodes on border.
  std::vector<size_t>      side_x;                                                                  // Nodes on "x" side.
  std::vector<size_t>      side_y;                                                                  // Nodes on "y" side.
  float                    x_min              = -1.0;                                               // "x_min" spatial boundary [m].
  float                    x_max              = +1.0;                                               // "x_max" spatial boundary [m].
  float                    y_min              = -1.0;                                               // "y_min" spatial boundary [m].
  float                    y_max              = +1.0;                                               // "y_max" spatial boundary [m].
  size_t                   side_x_nodes;                                                            // Number of nodes in "x" direction [#].
  size_t                   side_y_nodes;                                                            // Number of nodes in "x" direction [#].
  float                    dx;                                                                      // x-axis mesh spatial size [m].
  float                    dy;                                                                      // y-axis mesh spatial size [m].
  float                    dz;                                                                      // z-axis mesh spatial size [m].
  float1*                  freedom            = new float1 ();                                      // Freedom.
  int1*                    nearest            = new int1 ();                                        // Neighbour.
  int1*                    offset             = new int1 ();                                        // Offset.

  // SIMULATION PARAMETERS:
  float                    h                  = 0.01;                                               // Cloth's thickness [m].
  float                    rho                = 1000.0;                                             // Cloth's mass density [kg/m^3].
  float                    E                  = 100000.0;                                           // Cloth's Young modulus [kg/(m*s^2)].
  float                    mu                 = 700.0;                                              // Cloth's viscosity [Pa*s].
  float                    m;                                                                       // Cloth's mass [kg].
  float                    g                  = 9.81;                                               // External gravity field [m/s^2].
  float                    K;                                                                       // Cloth's elastic constant [kg/s^2].
  float                    B;                                                                       // Cloth's damping [kg*s*m].
  float                    dt_critical;                                                             // Critical time step [s].
  float                    dt_simulation;                                                           // Simulation time step [s].
  float1*                  dt                 = new float1 ();                                      // Time step [s].

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////// DATA INITIALIZATION //////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  object->init (bas, std::string (GMSH_HOME) + std::string (GMSH_MESH));                            // Initializing object mesh...
  nodes    = object->node.size ();                                                                  // Getting number of nodes...
  elements = object->element.size ();                                                               // Getting number of elements...

  color->init (nodes);                                                                              // Initializing color data...
  position->init (nodes);                                                                           // Initializing position data...
  position_int->init (nodes);                                                                       // Initializing intermediate position data...
  velocity->init (nodes);                                                                           // Initializing velocity data...
  velocity_int->init (nodes);                                                                       // Initializing intermediate position data...
  acceleration->init (nodes);                                                                       // Initializing acceleration data...
  acceleration_int->init (nodes);                                                                   // Initializing intermediate position data...
  gravity->init (1);                                                                                // Initializing gravity data...
  friction->init (1);                                                                               // Initializing friction data...
  mass->init (nodes);                                                                               // Initializing mass data...
  offset->init (nodes);                                                                             // Initializing node offset...
  freedom->init (nodes);                                                                            // Initializing freedom...
  dt->init (1);                                                                                     // Initializing time step data [s]...

  kernel_sx          = nodes;                                                                       // Setting OpenCL kernel "x" dimension...
  kernel_sy          = 0;                                                                           // Setting OpenCL kernel "y" dimension...
  kernel_sz          = 0;                                                                           // Setting OpenCL kernel "z" dimension...

  gravity->data[0].x = 0.0f;                                                                        // Setting gravity "x" component...
  gravity->data[0].y = 0.0f;                                                                        // Setting gravity "y" component...
  gravity->data[0].z = -g;                                                                          // Setting gravity "z" component...
  gravity->data[0].w = 1.0f;                                                                        // Setting gravity "w" component...

  neighbours         = 0;                                                                           // Initializing number of neighbours...

  for(i = 0; i < nodes; i++)
  {
    neighbourhood   = object->neighbours (i);                                                       // Getting neighbourhood...
    neighbours     += neighbourhood.size ();                                                        // Accumulating number of neighbours...
    offset->data[i] = neighbours;                                                                   // Setting neighbour offset...

    for(j = 0; j < neighbourhood.size (); j++)
    {
      neighbour.push_back (neighbourhood[j]);                                                       // Assembling neighbour tuple...
    }

    neighbourhood.clear ();
  }

  nearest->init (neighbours);                                                                       // Initializing neighbour...
  resting->init (neighbours);                                                                       // Initializing resiting position data...
  stiffness->init (neighbours);                                                                     // Initializing stiffness data...

  border            = object->physical (1, 1);                                                      // Getting nodes on border...
  border_nodes      = border.size ();                                                               // Getting number of nodes on border...
  side_x            = object->physical (1, 2);                                                      // Getting nodes on side_x...
  side_x_nodes      = side_x.size ();                                                               // Getting number of nodes on side_x...
  side_y            = object->physical (1, 3);                                                      // Getting nodes on side_y...
  side_y_nodes      = side_y.size ();                                                               // Getting number of nodes on side_y...
  dx                = (x_max - x_min)/(side_x_nodes - 1);                                           // x-axis mesh spatial size [m].
  dy                = (y_max - y_min)/(side_y_nodes - 1);                                           // y-axis mesh spatial size [m].
  dz                = dx;                                                                           // z-axis mesh spatial size [m].
  m                 = rho*h*dx*dy;                                                                  // Cloth's mass [kg].
  K                 = E*h*dy/dx;                                                                    // Cloth's elastic constant [kg/s^2].
  B                 = mu*h*dx*dy;                                                                   // Cloth's damping [kg*s*m].
  dt_critical       = sqrt (m/K);                                                                   // Critical time step [s].
  dt_simulation     = 0.8* dt_critical;                                                             // Simulation time step [s].
  friction->data[0] = B;                                                                            // Setting friction...
  dt->data[0]       = dt_simulation;                                                                // Setting time step...

  for(i = 0; i < nodes; i++)
  {
    color->data[i].x        = 0.01f*(rand () % 100);                                                // Setting "r" color coordinate...
    color->data[i].y        = 0.01f*(rand () % 100);                                                // Setting "g" color coordinate...
    color->data[i].z        = 0.01f*(rand () % 100);                                                // Setting "b" color coordinate...
    color->data[i].w        = 1.0f;                                                                 // Setting "a" color coordinate...

    position->data[i].x     = object->node[i].x;                                                    // Setting "x" position...
    position->data[i].y     = object->node[i].y;                                                    // Setting "y" position...
    position->data[i].z     = object->node[i].z;                                                    // Setting "z" position...
    position->data[i].w     = object->node[i].w;                                                    // Setting "w" position...

    velocity->data[i].x     = 0.0f;                                                                 // Setting "x" velocity...
    velocity->data[i].y     = 0.0f;                                                                 // Setting "y" velocity...
    velocity->data[i].z     = 0.0f;                                                                 // Setting "z" velocity...
    velocity->data[i].w     = 1.0f;                                                                 // Setting "w" velocity...

    acceleration->data[i].x = 0.0f;                                                                 // Setting "x" acceleration...
    acceleration->data[i].y = 0.0f;                                                                 // Setting "y" acceleration...
    acceleration->data[i].z = 0.0f;                                                                 // Setting "z" acceleration...
    acceleration->data[i].w = 1.0f;                                                                 // Setting "w" acceleration...

    mass->data[i]           = m;                                                                    // Setting "x" mass...

    freedom->data[i]        = 1.0f;                                                                 // Setting freedom...

    j_max                   = offset->data[i];

    if(i == 0)
    {
      j_min = 0;
    }
    else
    {
      j_min = offset->data[i - 1];
    }

    for(j = j_min; j < j_max; j++)
    {
      nearest->data[j]   = neighbour[j];                                                            // Setting neighbour tuple data...
      k                  = nearest->data[j];
      resting->data[j]   = sqrt (
                                 pow (position->data[k].x - position->data[i].x, 2) +
                                 pow (position->data[k].y - position->data[i].y, 2) +
                                 pow (position->data[k].z - position->data[i].z, 2)
                                );
      stiffness->data[j] = K;
    }
  }

  // Anchoring nodes on the border:
  for(int i = 0; i < border_nodes; i++)
  {
    freedom->data[i] = 0.0f;                                                                        // Setting freedom...
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////// NEUTRINO INITIALIZATION /////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  bas->init (QUEUE_NUM, KERNEL_NUM);                                                                // Initializing Neutrino baseline...
  gui->init
  (
   bas,                                                                                             // Neutrino baseline.
   GUI_SIZE_X,                                                                                      // GUI x-size [px].
   GUI_SIZE_Y,                                                                                      // GUI y-size [px.]
   GUI_NAME,                                                                                        // GUI name.
   orbit_x_init,                                                                                    // Initial x-orbit.
   orbit_y_init,                                                                                    // Initial y-orbit.
   pan_x_init,                                                                                      // Initial x-pan.
   pan_y_init,                                                                                      // Initial y-pan.
   pan_z_init                                                                                       // Initial z-pan.
  );
  ctx->init (bas, gui, NU_GPU);                                                                     // Initializing OpenCL context...
  S->init (bas, SHADER_HOME, SHADER_VERT, SHADER_GEOM, SHADER_FRAG);                                // Initializing OpenGL shader...
  Q->init (bas);                                                                                    // Initializing OpenCL queue...
  kernel_home = KERNEL_HOME;                                                                        // Setting kernel home directory...
  kernel_1.push_back ("thekernel1.cl");                                                             // Setting 1st source file...
  K1->init (bas, kernel_home, kernel_1, kernel_sx, kernel_sy, kernel_sz);                           // Initializing OpenCL kernel K1...
  kernel_2.push_back ("thekernel2.cl");                                                             // Setting 1st source file...
  K2->init (bas, kernel_home, kernel_2, kernel_sx, kernel_sy, kernel_sz);                           // Initializing OpenCL kernel K2...

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////// SETTING OPENCL KERNEL ARGUMENTS /////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  K1->setarg (color, 0);                                                                            // Setting color kernel argument...
  K1->setarg (position, 1);                                                                         // Setting position kernel argument...
  K1->setarg (position_int, 2);                                                                     // Setting intermediate position kernel argument...
  K1->setarg (velocity, 3);                                                                         // Setting velocity kernel argument...
  K1->setarg (velocity_int, 4);                                                                     // Setting intermediate velocity kernel argument...
  K1->setarg (acceleration, 5);                                                                     // Setting acceleration kernel argument...
  K1->setarg (acceleration_int, 6);                                                                 // Setting intermediate acceleration kernel argument...
  K1->setarg (gravity, 7);                                                                          // Setting gravity kernel argument...
  K1->setarg (stiffness, 8);                                                                        // Setting stiffness kernel argument...
  K1->setarg (resting, 9);                                                                          // Setting resting position kernel argument...
  K1->setarg (friction, 10);                                                                        // Setting friction kernel argument...
  K1->setarg (mass, 11);                                                                            // Setting mass kernel argument...
  K1->setarg (nearest, 12);                                                                         // Setting neighbour kernel argument...
  K1->setarg (offset, 13);                                                                          // Setting offset kernel argument...
  K1->setarg (freedom, 14);                                                                         // Setting freedom flag kernel argument...
  K1->setarg (dt, 15);                                                                              // Setting time step kernel argument...

  K2->setarg (color, 0);                                                                            // Setting color kernel argument...
  K2->setarg (position, 1);                                                                         // Setting position kernel argument...
  K2->setarg (position_int, 2);                                                                     // Setting intermediate position kernel argument...
  K2->setarg (velocity, 3);                                                                         // Setting velocity kernel argument...
  K2->setarg (velocity_int, 4);                                                                     // Setting intermediate velocity kernel argument...
  K2->setarg (acceleration, 5);                                                                     // Setting acceleration kernel argument...
  K2->setarg (acceleration_int, 6);                                                                 // Setting intermediate acceleration kernel argument...
  K2->setarg (gravity, 7);                                                                          // Setting gravity kernel argument...
  K2->setarg (stiffness, 8);                                                                        // Setting stiffness kernel argument...
  K2->setarg (resting, 9);                                                                          // Setting resting position kernel argument...
  K2->setarg (friction, 10);                                                                        // Setting friction kernel argument...
  K2->setarg (mass, 11);                                                                            // Setting mass kernel argument...
  K2->setarg (nearest, 12);                                                                         // Setting neighbour kernel argument...
  K2->setarg (offset, 13);                                                                          // Setting offset kernel argument...
  K2->setarg (freedom, 14);                                                                         // Setting freedom flag kernel argument...
  K2->setarg (dt, 15);                                                                              // Setting time step kernel argument...

  color->name    = "voxel_color";                                                                   // Setting variable name for OpenGL shader...
  position->name = "voxel_center";                                                                  // Setting variable name for OpenGL shader...

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////// WRITING DATA ON OPENCL QUEUE //////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  Q->write (color, 0);                                                                              // Writing color data on queue...
  Q->write (position, 1);                                                                           // Writing position data on queue...
  Q->write (position_int, 2);                                                                       // Writing intermediate position data on queue...
  Q->write (velocity, 3);                                                                           // Writing velocity data on queue...
  Q->write (velocity_int, 4);                                                                       // Writing intermediate velocity data on queue...
  Q->write (acceleration, 5);                                                                       // Writing acceleration data on queue...
  Q->write (acceleration_int, 6);                                                                   // Writing intermediate acceleration data on queue...
  Q->write (gravity, 7);                                                                            // Writing gravity data on queue...
  Q->write (stiffness, 8);                                                                          // Writing stiffness data on queue...
  Q->write (resting, 9);                                                                            // Writing resting position data on queue...
  Q->write (friction, 10);                                                                          // Writing friction data on queue...
  Q->write (mass, 11);                                                                              // Writing mass data on queue...
  Q->write (nearest, 12);                                                                           // Writing neighbour on queue...
  Q->write (offset, 13);                                                                            // Writing offset on queue...
  Q->write (freedom, 14);                                                                           // Writing freedom flag data on queue...
  Q->write (dt, 14);                                                                                // Writing time step data on queue...

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// SETTING OPENGL SHADER ARGUMENTS ////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  S->setarg (color, 0);                                                                             // Setting shader argument "0"...
  S->setarg (position, 1);                                                                          // Setting shader argument "1"...
  S->build ();                                                                                      // Building shader program...

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////// APPLICATION LOOP ////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  while(!gui->closed ())                                                                            // Opening window...
  {
    bas->get_tic ();                                                                                // Getting "tic" [us]...

    gui->clear ();                                                                                  // Clearing gui...
    gui->poll_events ();                                                                            // Polling gui events...

    Q->acquire (color, 0);                                                                          // Acquiring OpenGL/CL shared argument...
    Q->acquire (position, 1);                                                                       // Acquiring OpenGL/CL shared argument...
    ctx->execute (K1, Q, NU_WAIT);                                                                  // Executing OpenCL kernel...
    ctx->execute (K2, Q, NU_WAIT);                                                                  // Executing OpenCL kernel...
    Q->release (color, 0);                                                                          // Releasing OpenGL/CL shared argument...
    Q->release (position, 1);                                                                       // Releasing OpenGL/CL shared argument...

    gui->mouse_navigation (
                           mouse_orbit_rate,                                                        // Orbit angular rate coefficient [rev/s].
                           mouse_pan_rate,                                                          // Pan translation rate [m/s].
                           mouse_decaytime                                                          // Orbit low pass decay time [s].
                          );

    gui->gamepad_navigation (
                             gamepad_orbit_rate,                                                    // Orbit angular rate coefficient [rev/s].
                             gamepad_pan_rate,                                                      // Pan translation rate [m/s].
                             gamepad_decaytime,                                                     // Low pass filter decay time [s].
                             gamepad_deadzone                                                       // Gamepad joystick deadzone [0...1].
                            );

    if(gui->button_CROSS)
    {
      gui->close ();                                                                                // Closing gui...
    }

    gui->plot (S);                                                                                  // Plotting shared arguments...
    gui->refresh ();                                                                                // Refreshing gui...
    bas->get_toc ();                                                                                // Getting "toc" [us]...
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////// CLEANUP ////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  delete bas;                                                                                       // Deleting Neutrino baseline...
  delete gui;                                                                                       // Deleting OpenGL gui...
  delete ctx;                                                                                       // Deleting OpenCL context...

  delete color;                                                                                     // Deleting color data...
  delete position;                                                                                  // Deleting position data...
  delete position_int;                                                                              // Deleting intermediate position data...
  delete velocity;                                                                                  // Deleting velocity data...
  delete velocity_int;                                                                              // Deleting intermediate velocity data...
  delete acceleration;                                                                              // Deleting acceleration data...
  delete acceleration_int;                                                                          // Deleting intermediate acceleration data...
  delete gravity;                                                                                   // Deleting gravity data...
  delete stiffness;                                                                                 // Deleting stiffness data...
  delete resting;                                                                                   // Deleting resting data...
  delete friction;                                                                                  // Deleting friction data...
  delete mass;                                                                                      // Deleting mass data...
  delete nearest;                                                                                   // Deleting neighbours...
  delete offset;                                                                                    // Deleting offset...
  delete freedom;                                                                                   // Deleting freedom flag data...
  delete dt;                                                                                        // Deleting time step data...

  delete Q;                                                                                         // Deleting OpenCL queue...
  delete K1;                                                                                        // Deleting OpenCL kernel...
  delete K2;                                                                                        // Deleting OpenCL kernel...

  return 0;
}
