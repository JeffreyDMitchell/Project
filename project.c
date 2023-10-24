/*
 *  Lighting
 *
 *  Demonstrates basic lighting using a cube, sphere and icosahedron.
 *
 *  Key bindings:
 *  l          Toggles lighting
 *  a/A        Decrease/increase ambient light
 *  d/D        Decrease/increase diffuse light
 *  s/S        Decrease/increase specular light
 *  e/E        Decrease/increase emitted light
 *  n/N        Decrease/increase shininess
 *  F1         Toggle smooth/flat shading
 *  F2         Toggle local viewer mode
 *  F3         Toggle light distance (1/5)
 *  F8         Change ball increment
 *  F9         Invert bottom normal
 *  m          Toggles light movement
 *  []         Lower/rise light
 *  p          Toggles ortogonal/perspective projection
 *  o          Cycles through objects
 *  +/-        Change field of view of perspective
 *  x          Toggle axes
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view angle
 *  ESC        Exit
 */
#include "CSCIx229.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

#define SWORD_VERTS 128

struct param
{
   char name[32];
   double * val;
   double delta;
   double min;
   double max;
};

int axes=1;       //  Display axes
int mode=1;       //  Projection mode
int move=1;       //  Move light
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
int fov=55;       //  Field of view (for perspective)
int obj=0;        //  Scene/opbject selection
double asp=1;     //  Aspect ratio
double dim=6;     //  Size of world
// Light values
int light     =   1;  // Lighting
int one       =   1;  // Unit value
int distance  =   5;  // Light distance
int inc       =  10;  // Ball increment
int smooth    =   1;  // Smooth/Flat shading
int local     =   0;  // Local Viewer Model
int emission  =   0;  // Emission intensity (%)
double ambient   =  10;  // Ambient intensity (%)
double diffuse   =  50;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shiny   =   1;  // Shininess (value)
int zh        =  90;  // Light azimuth
float ylight  =   0;  // Elevation of light
typedef struct {float x,y,z;} vtx;
typedef struct {int A,B,C;} tri;

// TERRAIN GEN STUFF
double cam_x = 0.0001, cam_z = 0.0001;
double chunk_size = 50;
double render_dist_dbl = 1;
int chunk_res = 8;
double cam_y_offset = -20;


float cam_rot_speed = 2.0f;

int windowHeight = 0;

#define PARAM_CT 3
struct param params[PARAM_CT] = {
   {.name="ambient light", .val=&ambient, .delta=1, .min=0, .max=100},
   {.name="render dist", .val=&render_dist_dbl, .delta=1, .min=0, .max=10000},
   {.name="y offset", .val=&cam_y_offset, .delta=5, .min=-1000, .max=1000}
   // {.name="cam_z", .val=&cam_z, .delta=.1, .min=-100, .max=100}
};
int cursor = 0;

#define min(a,b) ((a > b) ? (b) : (a))
#define max(a,b) ((a > b) ? (a) : (b))

unsigned char keys[256];

#define n 500
vtx is[n];

double amod(double a, double b, double off)
{
   double r =  fmod(a+off, b);

   return ((r < 0) ? r+b : r)-off;
}

double omod(double a, double b)
{
   double r = fmod(a, b);

   return (r < 0) ? r+b : r;
}

void normalizeVector(GLfloat *v, GLfloat *dest) 
{
   float length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
   dest[0] = v[0] / length;
   dest[1] = v[1] / length;
   dest[2] = v[2] / length;
}

void crossProduct(GLfloat *v1, GLfloat *v2, GLfloat *dest) 
{
   dest[0] = v1[1] * v2[2] - v1[2] * v2[1];
   dest[1] = v1[2] * v2[0] - v1[0] * v2[2];
   dest[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void key_typed(unsigned char key,int x,int y)
{
   switch(key)
   {
      case 'L':
      case 'l': light=1-light; break;

      case 'M':
      case 'm': move=1-move; break;
   }
}

void keyboardDown(unsigned char key, int x, int y) 
{
   keys[key] = 1;

   key_typed(key, x , y);

   // Reproject
   Project(mode?fov:0,asp,dim);
   // Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

void keyboardUp(unsigned char key, int x, int y) 
{
   keys[key] = 0;

   // Reproject
   Project(mode?fov:0,asp,dim);
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

   if(keys['i']) cam_z-=10.0;
   if(keys['k']) cam_z+=10.0;
   if(keys['j']) cam_x-=10.0;
   if(keys['l']) cam_x+=10.0;

   // bounds checking
   if(ph >= 90) ph = 90;
   if(ph <= 0) ph = 0;
   if(fov >= 80) fov = 80;
   if(fov <= 20) fov = 20;
   if(th >= 360) th = 0;
   if(th < 0) th = 360;

   if(!move)
   {
      if(keys['[']) zh++;
      if(keys[']']) zh--;
   }
   
   Project(mode?fov:0,asp,dim);
}

static void cube(double x,double y, double z,
                 double dx, double dy, double dz,
                 double rx, double ry, double rz)
{
      //  Set specular color to white
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
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

/*
 *  Icosahedron data
 */
const int N=20;
//  Triangle index list
const tri idx[] =
   {
      { 2, 1, 0}, { 3, 2, 0}, { 4, 3, 0}, { 5, 4, 0}, { 1, 5, 0},
      {11, 6, 7}, {11, 7, 8}, {11, 8, 9}, {11, 9,10}, {11,10, 6},
      { 1, 2, 6}, { 2, 3, 7}, { 3, 4, 8}, { 4, 5, 9}, { 5, 1,10},
      { 2, 7, 6}, { 3, 8, 7}, { 4, 9, 8}, { 5,10, 9}, { 1, 6,10}
   };
//  Vertex coordinates
const vtx xyz[] =
   {
      { 0.000, 0.000, 1.000}, { 0.894, 0.000, 0.447}, { 0.276, 0.851, 0.447},
      {-0.724, 0.526, 0.447}, {-0.724,-0.526, 0.447}, { 0.276,-0.851, 0.447},
      { 0.724, 0.526,-0.447}, {-0.276, 0.851,-0.447}, {-0.894, 0.000,-0.447},
      {-0.276,-0.851,-0.447}, { 0.724,-0.526,-0.447}, { 0.000, 0.000,-1.000}
   };

// absolute dumpster fire.... 
// just constrain generated verts to [-.5, 0.5] on x and z


// static void drawChunk(double chunk_x,double y, double chunk_z)
// {
//    float white[] = {1,1,1,1};
//    float black[] = {0,0,0,1};
//    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
//    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
//    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

//    // Save transformation
//    glPushMatrix();
//    // Offset

//    float adj_x = (chunk_x * chunk_size) + cam_x;
//    float adj_z = (chunk_z * chunk_size) + cam_z;
//    double frag = 1.0 / chunk_res;

//    double half_chunk_size = chunk_size / 2.0;

//    glTranslated(adj_x,y,adj_z);
//    glScaled(chunk_size, chunk_size, chunk_size);

//    // chunk_res segments means chunk_res+1 verts i guess 
//    float chunk_verts[chunk_res+1][chunk_res+1];

//    for(int z = 0; z < chunk_res+1; z++)
//       for(int x = 0; x < chunk_res+1; x++)
//       {
//          chunk_verts[z][x] = sin((adj_x+(x / (double) chunk_res * chunk_size) - half_chunk_size) / 10.0) - sin((adj_z+(z / (double) chunk_res * chunk_size) - half_chunk_size) / 10.0);
//          chunk_verts[z][x] /= 2.0;
//       }

   
//    glBegin(GL_QUADS);
//    for(int z = 0; z < chunk_res; z++)
//       for(int x = 0; x < chunk_res; x++)
//       {
//          float x1 = (frag*x)-0.5;
//          float x2 = (frag*(x+1))-0.5;
//          float z1 = (frag*z)-0.5;
//          float z2 = (frag*(z+1))-0.5;


//          glVertex3f(x1,chunk_verts[z][x],z1);
//          glVertex3f(x1,chunk_verts[z+1][x],z2);
//          glVertex3f(x2,chunk_verts[z+1][x+1],z2);
//          glVertex3f(x2,chunk_verts[z][x+1],z1);
//       }
//    glEnd();

//    glPopMatrix();
// }

static void drawChunk(double screen_x,double y, double screen_z, int id_x, int id_z)
{

   // printf("idx: %d idz: %d\n", id_x, id_z);

   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

   // Save transformation
   glPushMatrix();
   // Offset
   float adj_x = id_x * chunk_size;
   float adj_z = id_z * chunk_size;
   double frag = 1.0 / chunk_res;

   double half_chunk_size = chunk_size / 2.0;

   glTranslated(screen_x,y,screen_z);
   glScaled(chunk_size, 1.0, chunk_size);

   // chunk_res segments means chunk_res+1 verts i guess 
   float chunk_verts[chunk_res+1][chunk_res+1];

   for(int z = 0; z < chunk_res+1; z++)
      for(int x = 0; x < chunk_res+1; x++)
      {
         float vert_x = adj_x+(x / (double) chunk_res * chunk_size) - half_chunk_size;
         float vert_z = adj_z+(z / (double) chunk_res * chunk_size) - half_chunk_size;

         // chunk_verts[z][x] = sin(vert_x / 10.0) - sin(vert_z / 10.0);
         // chunk_verts[z][x] *= 10.0;

         float s0 = 1.0f;
         float s1 = 0.1f;
         float s2 = 0.01f;
         float s3 = 0.001f;

         float sbiome = 0.0005f;
         
         chunk_verts[z][x] = 
            // "topography"
            (
            stb_perlin_noise3(vert_x * s1, 0, vert_z * s1, 0, 0, 0) * 5 + 
            stb_perlin_noise3(vert_x * s2, 1, vert_z * s2, 0, 0, 0) * 50 + 
            stb_perlin_noise3(vert_x * s3, 2, vert_z * s3, 0, 0, 0) * 100
            )
            // "biome" (hilly or flat)
            * (stb_perlin_noise3(vert_x * sbiome, 2, vert_z * sbiome, 0, 0, 0) + 1) * 3
         ;
      }

   
   glBegin(GL_QUADS);
   for(int z = 0; z < chunk_res; z++)
      for(int x = 0; x < chunk_res; x++)
      {
         float x1 = (frag*x)-0.5;
         float x2 = (frag*(x+1))-0.5;
         float z1 = (frag*z)-0.5;
         float z2 = (frag*(z+1))-0.5;

         glVertex3f(x1,chunk_verts[z][x],z1);
         glVertex3f(x1,chunk_verts[z+1][x],z2);
         glVertex3f(x2,chunk_verts[z+1][x+1],z2);
         glVertex3f(x2,chunk_verts[z][x+1],z1);
      }
   glEnd();

   glPopMatrix();
}

static void tile(float x, float y, float z, float size)
{
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

   glPushMatrix();

   glTranslated(x,y,z);
   glScaled(size, size, size);
   glBegin(GL_QUADS);

   glNormal3f(0.0, 1.0, 0.0);
   glVertex3f(-0.5, 0, -0.5);
   glVertex3f(0.5, 0, -0.5);
   glVertex3f(0.5, 0, 0.5);
   glVertex3f(-0.5, 0, 0.5);

   glEnd();

   glPopMatrix();
}

void display()
{
   processInput();

   glClearColor(ambient / 100.0, ambient / 100.0, ambient / 100.0, 1);
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

   glColor3f(1,1,1);
   // cube(0, 0, 0, 1, 1, 1, 0, 0, 0);

   // for fixed cam, maybe the fmod stuff was good? we didnt have tile updating based on "cam" position at that point
   int render_dist = (int) floor(render_dist_dbl);

   // double half_chunk_size = chunk_size / 2.0;
   // for(int x_chunk = -render_dist; x_chunk <= render_dist; x_chunk++)
   //    for(int z_chunk = -render_dist; z_chunk <= render_dist; z_chunk++)
   //    {

   //       int chunk_off_x = ceil((cam_x-half_chunk_size)/chunk_size);
   //       int chunk_off_z = ceil((cam_z-half_chunk_size)/chunk_size);

   //       // double x_val = (sin((cam_x + (x_chunk * chunk_size)) / 10.0) + 1) / 2.0;
   //       // double z_val = (sin((cam_z + (z_chunk * chunk_size)) / 10.0) + 1) / 2.0;
   //       // if((x_chunk + chunk_off_x + z_chunk + chunk_off_z) % 2)
   //       //    glColor3f(1-x_val,1-z_val,1);
   //       // else
   //       //    glColor3f(x_val,z_val,0);
   //       if((x_chunk + chunk_off_x + z_chunk + chunk_off_z) % 2)
   //          glColor3f(1,1,1);
   //       else
   //          glColor3f(0,0,0);

   //       // tile(
   //       //    x_chunk*chunk_size + amod(chunk_off_x * chunk_size - cam_x, chunk_size, half_chunk_size),
   //       //    -20, 
   //       //    z_chunk*chunk_size + amod(chunk_off_z * chunk_size - cam_z, chunk_size, half_chunk_size),
   //       //    chunk_size);

   //       // drawChunk(
   //       //    floor((cam_x + (x_chunk * chunk_size)) / chunk_size),
   //       //    -10,
   //       //    floor(cam_z + (z_chunk * chunk_size) / chunk_size)
   //       // );

   //       drawChunk(
   //          x_chunk*chunk_size + amod(chunk_off_x * chunk_size - cam_x, chunk_size, half_chunk_size),
   //          -10,
   //          z_chunk*chunk_size + amod(chunk_off_z * chunk_size - cam_z, chunk_size, half_chunk_size),
   //          (int) (floor((cam_x+half_chunk_size) / chunk_size) + x_chunk),
   //          (int) (floor((cam_z+half_chunk_size) / chunk_size) + z_chunk)
   //       );
   //    }

   double half_chunk_size = chunk_size / 2.0;

   int chunk_off_x = ceil((cam_x - half_chunk_size) / chunk_size);
   int chunk_off_z = ceil((cam_z - half_chunk_size) / chunk_size);

   for (int x_chunk = -render_dist; x_chunk <= render_dist; x_chunk++) {
      for (int z_chunk = -render_dist; z_chunk <= render_dist; z_chunk++) {
         int adjusted_x = x_chunk + chunk_off_x;
         int adjusted_z = z_chunk + chunk_off_z;

         // if ((adjusted_x + adjusted_z) % 2) {
         //       glColor3f(1, 1, 1);
         // } else {
         //       glColor3f(0, 0, 0);
         // }

         int chunk_world_x = (int) (floor((cam_x + half_chunk_size) / chunk_size) + x_chunk);
         int chunk_world_z = (int) (floor((cam_z + half_chunk_size) / chunk_size) + z_chunk);

         double x_val = sin((chunk_world_x / 100.1) + 1) / 2.0;
         double z_val = sin((chunk_world_z / 100.0) + 1) / 2.0;

         if((adjusted_x + adjusted_z) % 2)
            glColor3f(1-x_val,1-z_val,1);
         else
            glColor3f(x_val,z_val,0);

         drawChunk(
               x_chunk * chunk_size + amod(chunk_off_x * chunk_size - cam_x, chunk_size, half_chunk_size),
               cam_y_offset,
               z_chunk * chunk_size + amod(chunk_off_z * chunk_size - cam_z, chunk_size, half_chunk_size),
               chunk_world_x,
               chunk_world_z
         );
      }
   }


   // printf("\n\n");

   glColor3f(1,0,0);
   // cube(amod(cam_x, chunk_size, half_chunk_size), 0, amod(cam_z, chunk_size, half_chunk_size), .25, .25, .25, 0, 0, 0);
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
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   if(move)
      zh = fmod(90*t,360.0);
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
   Project(mode?fov:0,asp,dim);
   windowHeight = height;
}

int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);
   // glutSetOption(GLUT_MULTISAMPLE, 4);
   glutInitWindowSize(400,400);
   glutCreateWindow("Lighting");
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
   glutMainLoop();
   return 0;
}
