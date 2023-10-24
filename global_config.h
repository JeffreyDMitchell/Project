#define PARAM_CT 4


// TERRAIN GEN STUFF
double chunk_size = 50;
double render_dist_dbl = 1;
int chunk_res = 8;

// CAMERA STUFF
double cam_x = 0.0001, cam_z = 0.0001;
double cam_y_offset = -20;
double cam_speed = 5.0;
float cam_rot_speed = 2.0f;
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
int fov=55;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
double dim=6;     //  Size of world

//lighting stuff
double ambient   =  10;  // Ambient intensity (%)