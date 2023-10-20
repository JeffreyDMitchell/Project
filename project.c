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
double cam_x = 0, cam_z = 0;
double render_dist_dbl = 1;



float cam_rot_speed = 2.0f;

int windowHeight = 0;

#define PARAM_CT 2
struct param params[PARAM_CT] = {
   {.name="ambient light", .val=&ambient, .delta=1, .min=0, .max=100},
   {.name="render dist", .val=&render_dist_dbl, .delta=1, .min=0, .max=100}
   // {.name="cam_x", .val=&cam_x, .delta=.1, .min=-100, .max=100},
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

   if(keys['i']) cam_z-=0.1;
   if(keys['k']) cam_z+=0.1;
   if(keys['j']) cam_x-=0.1;
   if(keys['l']) cam_x+=0.1;

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


/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
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

   double chunk_size = 2.5;
   double half_chunk_size = chunk_size / 2.0;
   for(int x_chunk = -render_dist; x_chunk <= render_dist; x_chunk++)
      for(int z_chunk = -render_dist; z_chunk <= render_dist; z_chunk++)
      {

         int chunk_off_x = ceil((cam_x-half_chunk_size)/chunk_size);
         int chunk_off_z = ceil((cam_z-half_chunk_size)/chunk_size);

         if((x_chunk + chunk_off_x + z_chunk + chunk_off_z) % 2)
            glColor3f(1,1,1);
         else
            glColor3f(0,0,0);

         tile(
            x_chunk*chunk_size + amod(chunk_off_x * chunk_size - cam_x, chunk_size, half_chunk_size),
            0, 
            z_chunk*chunk_size + amod(chunk_off_z * chunk_size - cam_z, chunk_size, half_chunk_size),
            chunk_size);
      }

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
