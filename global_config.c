#include "global_config.h"

// TERRAIN GEN STUFF
double chunk_size = 8192;
// double render_dist_dbl = 3;
double water_level = 0;
int render_dist = 3;
int chunk_res_verts = CHUNK_RES;
int chunk_res_faces = CHUNK_RES-1;


// CAMERA STUFF
double cam_x = 0.0001, cam_z = 0.0001, cam_y=0.0001;
double cam_y_offset = 0;
double cam_speed = 50.0;
float cam_rot_speed = 2.0f;
float th=0;         //  Azimuth of view angle
float ph=0;         //  Elevation of view angle
int fov=55;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
double dim=6;     //  Size of world
double sens = 5;

// fpv
// double cam_pos[3] = {0.0f, 0.0f, 0.0f};
// double cam_front[3] = {0.0f, 0.0f, -1.0f};
// double cam_up[3] = {0.0f, 1.0f, 0.0f};

// window stuff
int wHeight = 600;
int wWidth = 600;
int cursorLock = 1;

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
int fog_enabled = 1;
float time_of_day = 0;