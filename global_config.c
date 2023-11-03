#include "global_config.h"

// TERRAIN GEN STUFF
double chunk_size = 4096;
double render_dist_dbl = 1;
double water_level = -580;
int chunk_res_verts = CHUNK_RES;
int chunk_res_faces = CHUNK_RES-1;


// CAMERA STUFF
double cam_x = 0.0001, cam_z = 0.0001, cam_y=0.0001;
double cam_y_offset = -1000;
double cam_speed = 10.0;
float cam_rot_speed = 2.0f;
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
int fov=55;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
double dim=6;     //  Size of world

//lighting stuff
// Light values
int light     =   1;  // Lighting
int one       =   1;  // Unit value
int distance  =   4096;  // Light distance
int inc       =  10;  // Ball increment
int smooth    =   1;  // Smooth/Flat shading
int local     =   0;  // Local Viewer Model
int emission  =   0;  // Emission intensity (%)
double ambient   =  10;  // Ambient intensity (%)
double diffuse   =  50;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shiny   =   1;  // Shininess (value)
float zh        =  90;  // Light azimuth
float ylight  =   0;  // Elevation of light