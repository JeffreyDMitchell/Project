#include <math.h>
#include <omp.h>

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
#include "CSCIx229.h"
#include "global_config.h"
#include "chunk.h"

// TODO remove
GLfloat fogColor[] = {0.7f, 0.7f, 0.7f, 1.0f};

typedef struct vtx
{
   float x,y,z;
} vtx;


struct param
{
   char name[32];
   double * val;
   double delta;
   double min;
   double max;
};

typedef struct bundle
{
   float mesh;
   vtx normal;
} bundle;

typedef struct chunk_t
{
   int id_x, id_z;
   // int hash;
   // combine for spacial locality reasons? idk man i just work here
   // float * mesh;
   // vtx * normals;
   bundle * bundles;
   // TODO
   // clutter stuff
} chunk_t;

#define CHUNK_CACHE_SIZE 64
chunk_t * chunk_cache[CHUNK_CACHE_SIZE][CHUNK_CACHE_SIZE];

void init()
{
   memset(chunk_cache, 0, sizeof(chunk_t *) * CHUNK_CACHE_SIZE * CHUNK_CACHE_SIZE);

   glEnable(GL_FOG);
   glFogfv(GL_FOG_COLOR, fogColor);
   glFogi(GL_FOG_MODE, GL_LINEAR);
   glFogf(GL_FOG_START, 4000.0f);  // Where the fog starts
   glFogf(GL_FOG_END, 5000.0f);   // Where the fog completely obscures objects
   glHint(GL_FOG_HINT, GL_NICEST);
   // glFogf (GL_FOG_DENSITY, 0.0005f);

   // printf("Size of struct bundle: %zu\n", sizeof(bundle));

   // printf("starting memory stress test\n");
   // for(int i = 0; i < CHUNK_CACHE_SIZE; i++)
   // {
   //    for(int j = 0; j < CHUNK_CACHE_SIZE; j++)
   //    {
   //          chunk_t * chunk = malloc(sizeof(chunk_t));
   //          initChunk(chunk, i, j);
   //          generateChunk(chunk);
   //          cache_chunk(chunk);
   //    }
   // }
   // printf("memory stress test succeeded\n");
}

inline void flushChunkCache()
{
   for(int i = 0; i < CHUNK_CACHE_SIZE; i++)
      for(int j = 0; j < CHUNK_CACHE_SIZE; j++)
         if(chunk_cache[i][j])
            destroyChunk(chunk_cache[i][j]);

   memset(chunk_cache, 0, sizeof(chunk_t *) * CHUNK_CACHE_SIZE * CHUNK_CACHE_SIZE);
}

inline double amod(double a, double b, double off)
{
   double r =  fmod(a+off, b);

   return ((r < 0) ? r+b : r)-off;
}

inline double omod(double a, double b)
{
   double r = fmod(a, b);

   return (r < 0) ? r+b : r;
}

inline int imod(int a, int b)
{
   int r = a % b;

   return (r < 0) ? r+b : r;
}

inline float smoothstep(float edge0, float edge1, float x) 
{
    float t = fminf(fmaxf((x - edge0) / (edge1 - edge0), 0.0f), 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

inline void normalizeVector(vtx *v) 
{
   float length = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
   v->x /= length;
   v->y /= length;
   v->z /= length;
}

inline void crossProduct(vtx *v1, vtx *v2, vtx *dest) 
{
   dest->x = v1->y * v2->z - v1->z * v2->y;
   dest->y = v1->z * v2->x - v1->x * v2->z;
   dest->z = v1->x * v2->y - v1->y * v2->x;
}

inline void initChunk(chunk_t * chunk, int id_x, int id_z)
{
   chunk->id_x = id_x;
   chunk->id_z = id_z;

   // chunk->mesh = malloc(sizeof(float) * (chunk_res_verts) * (chunk_res_verts));
   // chunk->normals = malloc(sizeof(vtx) * chunk_res_verts * chunk_res_verts);
   chunk->bundles = malloc(sizeof(bundle) * chunk_res_verts * chunk_res_verts);
}


inline void destroyChunk(chunk_t * chunk)
{
   // free(chunk->mesh);
   // free(chunk->normals);
   free(chunk->bundles);
   free(chunk);
}

inline void cache_chunk(chunk_t * chunk)
{
   int id_x = chunk->id_x;
   int id_z = chunk->id_z;

   chunk_t ** target = &chunk_cache[imod(id_z, CHUNK_CACHE_SIZE)][imod(id_x, CHUNK_CACHE_SIZE)];

   if(*target) destroyChunk(*target);

   *target = chunk;
}

inline chunk_t * get_chunk(int id_x, int id_z)
{
   chunk_t * fetched = chunk_cache[imod(id_z, CHUNK_CACHE_SIZE)][imod(id_x, CHUNK_CACHE_SIZE)];

   if(fetched && !(fetched->id_x == id_x && fetched->id_z == id_z))
      return (chunk_t *) NULL;   

   return fetched;
}

void generateChunk(chunk_t * chunk)
{
   double frag = 1.0 / chunk_res_faces;
   float adj_x = chunk->id_x * chunk_size;
   float adj_z = chunk->id_z * chunk_size;
   double half_chunk_size = chunk_size / 2.0;
   
   #pragma omp parallel for collapse(2)
   for(int z = 0; z < chunk_res_verts; z++)
      for(int x = 0; x < chunk_res_verts; x++)
      {
         float vert_x = adj_x+(x / (double) chunk_res_faces * chunk_size) - half_chunk_size;
         float vert_z = adj_z+(z / (double) chunk_res_faces * chunk_size) - half_chunk_size;

         float s0 = 1.0f;
         float s1 = 0.1f;
         float s2 = 0.004f;
         float s3 = 0.002f;
         float s4 = 0.0005f;

         float sbiome = 0.0001f;
         
         // chunk->mesh[(z * (chunk_res_verts)) + x] = 
         //    // "topography"
         //    (
         //       // (stb_perlin_noise3(vert_x * s1, 0, vert_z * s1, 0, 0, 0) + 1) * 5 + 
         //       (stb_perlin_noise3(vert_x * s2, 1, vert_z * s2, 0, 0, 0) + 1) * 50 + 
         //       (stb_perlin_noise3(vert_x * s3, 2, vert_z * s3, 0, 0, 0) + 1) * 100
         //    )
         //    // "biome" (hilly or flat)
         //    * smoothstep(0.05,0.95,stb_perlin_noise3(vert_x * sbiome, 2, vert_z * sbiome, 0, 0, 0) + 1) * 3
         // ;
         float base = (stb_perlin_noise3(vert_x * s4, 0, vert_z * s4, 0, 0, 0) + 1) / 2.0;
         float hills = base * 1000;
         float mountains = smoothstep(.5,.95,base) * pow((stb_perlin_noise3(vert_x * s2, 1, vert_z * s2, 0, 0, 0) + 1) / 2.0 * 2, 3) * 100;
         float lakes = smoothstep(0.5,0.95,1.0-base) * (stb_perlin_noise3(vert_x * s3, 2, vert_z * s3, 0, 0, 0) - 1) / 2.0 * 500;

         chunk->bundles[(z * (chunk_res_verts)) + x].mesh = 
            hills + 
            mountains + 
            lakes +

            cam_y_offset
         ;
      }

   // generate quads on per-face basis
   vtx face_norms[chunk_res_faces][chunk_res_faces];
   #pragma omp parallel for collapse(2)
   for(int z = 0; z < chunk_res_faces; z++)
      for(int x = 0; x < chunk_res_faces; x++)
      {
         vtx e1, e2, norm;
         // normals must mirror geometry as it would be drawn...
         float x1 = (frag*x)-0.5;
         float x2 = (frag*(x+1))-0.5;
         float z1 = (frag*z)-0.5;
         float z2 = (frag*(z+1))-0.5;

         bundle * bundles = chunk->bundles;

         e1.x = 0; e1.z = z2-z1; e1.y = bundles[((z+1) * (chunk_res_verts)) + x].mesh - bundles[(z * (chunk_res_verts)) + x].mesh;
         e2.x = x2-x1; e2.z = 0; e2.y = bundles[((z+1) * (chunk_res_verts)) + (x+1)].mesh - bundles[((z+1) * (chunk_res_verts)) + x].mesh;

         crossProduct(&e1, &e2, &norm);
         // TODO remove
         normalizeVector(&norm);

         face_norms[z][x] = norm;
      }

   // coalesce into per-vert normals
   #pragma omp parallel for collapse(2)
   for(int z = 0; z < chunk_res_verts; z++)
      for(int x = 0; x < chunk_res_verts; x++)
      {
         vtx norm = {0.0,0.0,0.0};

         for(int z_off = -1; z_off <= 1; z_off++)
            for(int x_off = -1; x_off <= 1; x_off++)
            {
               int eff_x = x + x_off;
               int eff_z = z + z_off;
               
               // oob, skip to next normal
               if(eff_x < 0 || eff_z < 0 || eff_x >= chunk_res_faces || eff_z >= chunk_res_faces) continue;

               vtx * cur = &face_norms[eff_z][eff_x];
               norm.x += cur->x;
               norm.y += cur->y;
               norm.z += cur->z;
            }
         normalizeVector(&norm);
         chunk->bundles[(z * (chunk_res_verts)) + x].normal = norm;
      }

}

struct param params[PARAM_CT] = {
   {.name="ambient light", .val=&ambient, .delta=1, .min=0, .max=100},
   {.name="render dist", .val=&render_dist_dbl, .delta=1, .min=0, .max=10000},
   {.name="y offset", .val=&cam_y_offset, .delta=5, .min=-1000, .max=1000},
   {.name="speed", .val=&cam_speed, .delta=5.0, .min=-.1, .max=1000},
   {.name="water level", .val=&water_level, .delta=5.0, .min=-1000, .max=1000},
};
int cursor = 0;

#define min(a,b) ((a > b) ? (b) : (a))
#define max(a,b) ((a > b) ? (a) : (b))

unsigned char keys[256];

void key_typed(unsigned char key,int x,int y)
{

}

void keyboardDown(unsigned char key, int x, int y) 
{
   keys[key] = 1;

   key_typed(key, x , y);

   // Reproject
   Project(fov,asp,dim);
   // Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

void keyboardUp(unsigned char key, int x, int y) 
{
   keys[key] = 0;

   // Reproject
   Project(fov,asp,dim);
   // Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

void processInput() 
{
   // bird's eye view cam controls
   if(keys['w']) ph -= cam_rot_speed;
   if(keys['s']) ph += cam_rot_speed;
   if(keys['a']) th -= cam_rot_speed;
   if(keys['d']) th += cam_rot_speed;
   if(keys['q']) fov--;
   if(keys['e']) fov++;

   if(keys['i']) cam_z-=cam_speed;
   if(keys['k']) cam_z+=cam_speed;
   if(keys['j']) cam_x-=cam_speed;
   if(keys['l']) cam_x+=cam_speed;

   // bounds checking
   if(ph >= 90) ph = 90;
   if(ph <= -90) ph = -90;
   if(fov >= 80) fov = 80;
   if(fov <= 20) fov = 20;
   if(th >= 360) th = 0;
   if(th < 0) th = 360;
   
   Project(fov,asp,dim);
}

static void cube(double x,double y, double z,
                 double dx, double dy, double dz,
                 double rx, double ry, double rz)
{
      //  Set specular color to white
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   // glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

   // Save transformation
   glPushMatrix();
   // Offset
   glTranslated(x,y,z);
   glRotated(rx,1,0,0);
   glRotated(ry,0,1,0);
   glRotated(rz,0,0,1);
   glScaled(dx,dy,dz);
   // Cube
   glBegin(GL_QUADS);
   // Front
   // glColor3f(1,0,0);
   glNormal3f( 0, 0, 1);
   glVertex3f(-1,-1, 1);
   glVertex3f(+1,-1, 1);
   glVertex3f(+1,+1, 1);
   glVertex3f(-1,+1, 1);
   // Back
   // glColor3f(0,0,1);
   glNormal3f( 0, 0,-1);
   glVertex3f(+1,-1,-1);
   glVertex3f(-1,-1,-1);
   glVertex3f(-1,+1,-1);
   glVertex3f(+1,+1,-1);
   // Right
   // glColor3f(1,1,0);
   glNormal3f( 1, 0, 0);
   glVertex3f(+1,-1,+1);
   glVertex3f(+1,-1,-1);
   glVertex3f(+1,+1,-1);
   glVertex3f(+1,+1,+1);
   // Left
   // glColor3f(0,1,0);
   glNormal3f(-1, 0, 0);
   glVertex3f(-1,-1,-1);
   glVertex3f(-1,-1,+1);
   glVertex3f(-1,+1,+1);
   glVertex3f(-1,+1,-1);
   // Top
   // glColor3f(0,1,1);
   glNormal3f( 0,+1, 0);
   glVertex3f(-1,+1,+1);
   glVertex3f(+1,+1,+1);
   glVertex3f(+1,+1,-1);
   glVertex3f(-1,+1,-1);
   // Bottom
   // glColor3f(1,0,1);
   glNormal3f( 0,-1, 0);
   glVertex3f(-1,-1,-1);
   glVertex3f(+1,-1,-1);
   glVertex3f(+1,-1,+1);
   glVertex3f(-1,-1,+1);
   // End
   glEnd();
   // Undo transofrmations
   glPopMatrix();
}


inline void drawChunk(chunk_t * chunk, double screen_x, double y, double screen_z, int id_x, int id_z)
{
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

   // Save transformation
   glPushMatrix();
   // float adj_x = id_x * chunk_size;
   // float adj_z = id_z * chunk_size;
   double frag = 1.0 / chunk_res_faces;

   // double half_chunk_size = chunk_size / 2.0;

   glTranslated(screen_x,y,screen_z);
   glScaled(chunk_size, 1.0, chunk_size);
   
   for(int z = 0; z < chunk_res_faces; z++)
   {
      glBegin(GL_QUAD_STRIP);
      for(int x = 0; x < chunk_res_verts; x++)
      {
         float x1 = (frag*x)-0.5;
         // float x2 = (frag*(x+1))-0.5;
         float z1 = (frag*z)-0.5;
         float z2 = (frag*(z+1))-0.5;

         bundle * bundles = chunk->bundles;
         vtx norm;
         int idx;

         // TODO completely rework this color stuff...

         // const float sand[] = {0.810, 0.778, 0.429};
         // const float grass[] = {0.245, 0.650, 0.208};
         
         
         idx = (z * (chunk_res_verts)) + x;
         norm = bundles[idx].normal;
         // glColor3fv((mesh[idx] > water_level + 10) ? grass : sand);
         glNormal3f(norm.x, norm.y, norm.z);
         glVertex3f(x1,bundles[idx].mesh,z1);

         idx = ((z+1) * (chunk_res_verts)) + x;
         norm = bundles[idx].normal;
         // glColor3fv((mesh[idx] > water_level + 10) ? grass : sand);
         glNormal3f(norm.x, norm.y, norm.z);
         glVertex3f(x1,bundles[idx].mesh,z2);

         // idx = ((z+1) * (chunk_res_verts)) + (x+1);
         // norm = chunk->bundles[idx].normal;
         // // glColor3fv((mesh[idx] > water_level + 10) ? grass : sand);
         // glNormal3f(norm.x, norm.y, norm.z);
         // glVertex3f(x2,bundles[idx].mesh,z2);

         // idx = (z * (chunk_res_verts)) + (x+1);
         // norm = chunk->bundles[idx].normal;
         // // glColor3fv((mesh[idx] > water_level + 10) ? grass : sand);
         // glNormal3f(norm.x, norm.y, norm.z);
         // glVertex3f(x2,bundles[idx].mesh,z1);
      }
      glEnd();
   }
   
   glPopMatrix();
}

static void Vertex(double th,double ph)
{
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glNormal3d(x,y,z);
   glVertex3d(x,y,z);
}

static void ball(double x,double y,double z,double r)
{
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball with yellow specular
   float yellow[]   = {1.0,1.0,0.0,1.0};
   float Emission[] = {0.0,0.0,0.01*emission,1.0};
   glColor3f(1,1,1);
   glMaterialf(GL_FRONT,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
   //  Bands of latitude
   for (int ph=-90;ph<90;ph+=inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (int th=0;th<=360;th+=2*inc)
      {
         Vertex(th,ph);
         Vertex(th,ph+inc);
      }
      glEnd();
   }
   //  Undo transofrmations
   glPopMatrix();
}
















void display()
{
   processInput();

   glClearColor(fogColor[0], fogColor[1], fogColor[2], 1.0);
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_MULTISAMPLE);

   //  Undo previous transformations
   glLoadIdentity();

   //  Perspective - set eye position
   double Ex = -2*dim*Sin(th)*Cos(ph);
   double Ey = +2*dim        *Sin(ph);
   double Ez = +2*dim*Cos(th)*Cos(ph);
   gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);

   glShadeModel(GL_SMOOTH);

      //  Light switch
   if (light)
   {
      //  Translate intensity to color vectors
      float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
      float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
      float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
      //  Light position
      float Position[]  = {0.0,distance*Sin(zh),distance*Cos(zh),0.0};
      //  Draw light position as ball (still no lighting here)
      glColor3f(1,1,1);

      // TODO: draw behind stuff
      ball(Position[0],Position[1],Position[2] , 100);
      //  OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      //  Enable lighting
      glEnable(GL_LIGHTING);
      //  Location of viewer for specular calculations
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      //  Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
   }
   else
      glDisable(GL_LIGHTING);
   

























   // for fixed cam, maybe the fmod stuff was good? we didnt have tile updating based on "cam" position at that point
   int render_dist = (int) floor(render_dist_dbl);

   double half_chunk_size = chunk_size / 2.0;

   int chunk_off_x = ceil((cam_x - half_chunk_size) / chunk_size);
   int chunk_off_z = ceil((cam_z - half_chunk_size) / chunk_size);

   for (int x_chunk = -render_dist; x_chunk <= render_dist; x_chunk++) {
      for (int z_chunk = -render_dist; z_chunk <= render_dist; z_chunk++) {
         // int adjusted_x = x_chunk + chunk_off_x;
         // int adjusted_z = z_chunk + chunk_off_z;

         // if ((adjusted_x + adjusted_z) % 2) {
         //       glColor3f(1, 1, 1);
         // } else {
         //       glColor3f(0, 0, 0);
         // }

         int chunk_world_x = (int) (floor((cam_x + half_chunk_size) / chunk_size) + x_chunk);
         int chunk_world_z = (int) (floor((cam_z + half_chunk_size) / chunk_size) + z_chunk);

         // double x_val = sin((chunk_world_x * 5.0) + 1) / 2.0;
         // double z_val = cos((chunk_world_z * 2.5) + 1) / 2.0;

         // if((adjusted_x + adjusted_z) % 2)
         //    glColor3f(1-x_val,1-z_val,1);
         // else
         //    glColor3f(x_val,z_val,0);

         glColor3f(0.245, 0.650, 0.208);

         chunk_t * chunk = get_chunk(chunk_world_x, chunk_world_z);

         if(!chunk)
         {
            chunk = malloc(sizeof(chunk_t));
            initChunk(chunk, chunk_world_x, chunk_world_z);
            generateChunk(chunk);
            cache_chunk(chunk);
            // printf("generating chunk.\n");
         }

         drawChunk(
               chunk,
               x_chunk * chunk_size + amod(chunk_off_x * chunk_size - cam_x, chunk_size, half_chunk_size),
               0,
               z_chunk * chunk_size + amod(chunk_off_z * chunk_size - cam_z, chunk_size, half_chunk_size),
               chunk_world_x,
               chunk_world_z
         );
         // destroyChunk(chunk);
      }
   }

   // draw waterline
   // Enable blending
   glEnable(GL_BLEND);
   // Set the blending function
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   float center_x = amod(chunk_off_x * chunk_size - cam_x, chunk_size, half_chunk_size);
   float center_z = amod(chunk_off_z * chunk_size - cam_z, chunk_size, half_chunk_size);
   int it_z[] = {-1,-1,+1,+1};
   int it_x[] = {-1,+1,+1,-1};
   glColor4f(0.0,0.0,1.0,0.5);
   glNormal3f(0.0,1.0,0.0);
   glBegin(GL_QUADS);
   for(int i = 0; i < 4; i++)
   {
      float x = center_x + (it_x[i] * (chunk_size * ((render_dist*2)+1) / 2.0));
      float z = center_z + (it_z[i] * (chunk_size * ((render_dist*2)+1) / 2.0));
      glVertex3f(x, water_level, z);
   }
   glEnd();

   glDisable(GL_BLEND);

   // draw "player"
   glColor3f(1,0,0);
   cube(0, 0, 0, .25, .25, .25, 0, 0, 0);

   glDisable(GL_LIGHTING);
   glColor3f(1,1,1);

   // display parameters
   for(int i = 0; i < PARAM_CT; i++)
   {
      glColor3f(1,1,1);
      glWindowPos2i(5,5+(20*i));
      Print("%s%s=%.1f", (cursor == i ? "->" : "  "), params[i].name, *params[i].val);
   }

   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

void idle()
{
   double t = glutGet(GLUT_ELAPSED_TIME) / 100.0;
   zh = fmod(t,210.0)-15;
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

void special(int key,int x,int y)
{
      switch(key)
   {
      case GLUT_KEY_UP: 
      cursor++; 
      if(cursor >= PARAM_CT)
         cursor = 0;
       break;

      case GLUT_KEY_DOWN:
         cursor--; 
         if(cursor < 0)
            cursor = PARAM_CT-1;
       break;

      case GLUT_KEY_LEFT: *params[cursor].val -= params[cursor].delta; break;
      case GLUT_KEY_RIGHT: *params[cursor].val += params[cursor].delta; break;
   }

   // clamp values
   if(*params[cursor].val < params[cursor].min) *params[cursor].val = params[cursor].min;
   if(*params[cursor].val > params[cursor].max) *params[cursor].val = params[cursor].max;

   // Update projection
   Project(fov,asp,dim);
}

void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, RES*width,RES*height);
   //  Set projection
   Project(fov,asp,dim);
}

int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);
   // glutSetOption(GLUT_MULTISAMPLE, 4);
   glutInitWindowSize(600,600);
   glutCreateWindow("Landscapes!");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(keyboardDown);
   glutKeyboardUpFunc(keyboardUp);
   glutIdleFunc(idle);
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   init();
   glutMainLoop();
   return 0;
}