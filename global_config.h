#ifndef _GLOBAL_CONFIG_H_
#define _GLOBAL_CONFIG_H_

#define PARAM_CT 6
#define CHUNK_RES 256
#define CHUNK_CACHE_SIZE 32

#define TWO_PI 6.283185307179586

// TERRAIN GEN STUFF
extern double chunk_size;
// extern double render_dist_dbl;
extern double water_level;
extern int render_dist;
extern int chunk_res_verts;
extern int chunk_res_faces;

// CAMERA STUFF
extern double cam_x;
extern double cam_z;
extern double cam_y;
extern double cam_y_offset;
extern double cam_speed;
extern float cam_rot_speed;
extern float th;
extern float ph;
extern int fov;
extern double asp;
extern double dim;
extern double sens;
// fpv
// extern vtx cam_pos;
// extern vtx cam_front;
// extern vtx cam_up;

// window stuff
extern int wHeight;
extern int wWidth;
extern int cursorLock;

//lighting stuff
extern int light;
extern int one;
extern int distance;
extern int inc;
extern int smooth;
extern int local;
extern int emission;
extern double ambient;
extern double diffuse;
extern int specular;
extern int shininess;
extern float shiny;
extern float zh;
extern float ylight;
extern int fog_enabled;
extern float time_of_day;

#endif