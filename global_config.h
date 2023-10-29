#ifndef _GLOBAL_CONFIG_H_
#define _GLOBAL_CONFIG_H_

#define PARAM_CT 5
#define CHUNK_RES 256

// TERRAIN GEN STUFF
extern double chunk_size;
extern double render_dist_dbl;
extern double water_level;
extern int chunk_res_verts;
extern int chunk_res_faces;

// CAMERA STUFF
extern double cam_x;
extern double cam_z;
extern double cam_y_offset;
extern double cam_speed;
extern float cam_rot_speed;
extern int th;
extern int ph;
extern int fov;
extern double asp;
extern double dim;

//lighting stuff
// Light values
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

#endif