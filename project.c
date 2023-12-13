#include <math.h>
#include <omp.h>

#include "CSCIx229.h"
#include "graphics_utils.h"
#include "global_config.h"
#include "chunk.h"
#include "parameter.h"

// TODO remove
GLfloat fogColor[] = {0.7f, 0.7f, 0.7f, 1.0f};
int lastX, lastY = 0;
vtx cam_pos = {0.0f, 0.0f, 10.0f};
vtx cam_front = {0.0f, 0.0f, -1.0f};
vtx cam_up = {0.0f, 1.0f, 0.0f};

chunk_t * chunk_cache[CHUNK_CACHE_SIZE][CHUNK_CACHE_SIZE];

void configureFog(int dist)
{
   // TODO this is cringe change it up
   if(fog_enabled)
   {
      glEnable(GL_FOG);
      glFogf(GL_FOG_START, (render_dist-1 > 0 ? render_dist-1 : 0) * chunk_size);
      glFogf(GL_FOG_END, render_dist * chunk_size);

      // glEnable(GL_FOG);
      // glFogi(GL_FOG_MODE, GL_EXP2);
      // glFogf(GL_FOG_DENSITY, 0.00005f);

   }
   else
   {
      glDisable(GL_FOG);
   }
}

void init()
{
   memset(chunk_cache, 0, sizeof(chunk_t *) * CHUNK_CACHE_SIZE * CHUNK_CACHE_SIZE);

   glEnable(GL_FOG);
   glFogfv(GL_FOG_COLOR, fogColor);
   glFogi(GL_FOG_MODE, GL_LINEAR);
   glHint(GL_FOG_HINT, GL_NICEST);
   glFogf (GL_FOG_DENSITY, 0.0005f);

   configureFog(render_dist);

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

// custom param functions
// void cursorLockChange()
// {

// }

void cursorLockChangeParam(struct param * self)
{
   if(*(int *)self->val)
      glutSetCursor(GLUT_CURSOR_NONE);
   else
      glutSetCursor(GLUT_CURSOR_INHERIT);
}

struct param params[PARAM_CT] = {
   {
      .name = "render distance",
      .type = INT_T,
      .val = &render_dist,
      .delta.i = 1,
      .min.i = 1,
      .max.i = 10,
      .incr = &intIncr,
      .toStr = &intToStr,
      .onChange = NULL
   },
   {
      .name = "speed",
      .type = DOUBLE_T,
      .val = &cam_speed,
      .delta.d = 5,
      .min.d = 0.0,
      .max.d = 100.0,
      .incr = &doubleIncr,
      .toStr = &doubleToStr,
      .onChange = NULL
   },
   {
      .name = "water level",
      .type = DOUBLE_T,
      .val = &water_level,
      .delta.d = 10.0,
      .min.d = -10000.0,
      .max.d = 10000.0,
      .incr = &doubleIncr,
      .toStr = &doubleToStr,
      .onChange = NULL
   },
   {
      .name = "fog enabled",
      .type = BOOL_T,
      .val = &fog_enabled,
      .delta.i = 1,
      .incr = &boolIncr,
      .toStr = &boolToStr,
      .onChange = NULL
   },
   {
      .name = "mouse control",
      .type = BOOL_T,
      .val = &cursorLock,
      .delta.i = 1,
      .incr = &boolIncr,
      .toStr = &boolToStr,
      .onChange = &cursorLockChangeParam
   },
   {
      .name = "mouse sensitivity",
      .type = DOUBLE_T,
      .val = &sens,
      .delta.d = 0.5,
      .min.d = 0.5,
      .max.d = 10.0,
      .incr = &doubleIncr,
      .toStr = &doubleToStr,
      .onChange = NULL
   }
};
int cursor = 0;

#define min(a,b) ((a > b) ? (b) : (a))
#define max(a,b) ((a > b) ? (a) : (b))

unsigned char keys[256];

void keyTyped(unsigned char key,int x,int y)
{
   switch(key)
   {
      case 27:
         // repeat garbage
         cursorLock = 0;
         glutSetCursor(GLUT_CURSOR_INHERIT);
      break;
   }
}

void keyboardDown(unsigned char key, int x, int y) 
{
   keys[key] = 1;

   keyTyped(key, x , y);

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

void mouseMove(int x, int y)
{
   if(!cursorLock) return;

   int cx = wWidth / 2;
   int cy = wHeight / 2;

   int dx = x - cx;
   int dy = y - cy;

   if ((dx || dy)) 
   {
      ph -= dy * cam_rot_speed * sens * 0.001;
      th += dx * cam_rot_speed * sens * 0.001;
      // TODO test on other platforms?
      glutWarpPointer(cx, cy);
   }
}

void processInput() 
{
   // // bird's eye view cam controls
   // if(keys['i']) ph -= cam_rot_speed;
   // if(keys['k']) ph += cam_rot_speed;
   // if(keys['j']) th -= cam_rot_speed;
   // if(keys['l']) th += cam_rot_speed;
   // if(keys['[']) fov--;
   // if(keys[']']) fov++;

   // if(keys['w']) cam_pos.z-=cam_speed;
   // if(keys['s']) cam_pos.z+=cam_speed;
   // if(keys['a']) cam_pos.x-=cam_speed;
   // if(keys['d']) cam_pos.x+=cam_speed;

   // if(keys['q']) cam_pos.y-=cam_speed;
   // if(keys['e']) cam_pos.y+=cam_speed;

   // // bounds checking
   // if(ph >= 90) ph = 90;
   // if(ph <= -90) ph = -90;
   // if(fov >= 80) fov = 80;
   // if(fov <= 20) fov = 20;
   // if(th >= 360) th = 0;
   // if(th < 0) th = 360;
   
   // Project(fov,asp,dim);

   // printf("x %f y %f z %f\n", cam_pos.x, cam_pos.y, cam_pos.z);

   if(keys['w']) 
   {
      cam_pos.x += cam_speed * cam_front.x;
      cam_pos.y += cam_speed * cam_front.y;
      cam_pos.z += cam_speed * cam_front.z;
   }
   if(keys['s']) 
   {
      cam_pos.x -= cam_speed * cam_front.x;
      cam_pos.y -= cam_speed * cam_front.y;
      cam_pos.z -= cam_speed * cam_front.z;
   }
   if(keys['a']) 
   {
      vtx cross;
      crossProduct(&cam_front, &cam_up, &cross);
      normalizeVector(&cross);
      cam_pos.x -= cam_speed * cross.x;
      cam_pos.y -= cam_speed * cross.y;
      cam_pos.z -= cam_speed * cross.z;
   }
   if(keys['d']) 
   {
      vtx cross;
      crossProduct(&cam_front, &cam_up, &cross);
      normalizeVector(&cross);
      cam_pos.x += cam_speed * cross.x;
      cam_pos.y += cam_speed * cross.y;
      cam_pos.z += cam_speed * cross.z;
   }

   if(keys['q']) cam_pos.y -= cam_speed;
   if(keys['e']) cam_pos.y += cam_speed;

   if(keys['i']) ph += cam_rot_speed;
   if(keys['k']) ph -= cam_rot_speed;
   if(keys['l']) th += cam_rot_speed;
   if(keys['j']) th -= cam_rot_speed;

   if(ph > 89.0f) ph = 89.0f;
   if(ph < -89.0f) ph = -89.0f;

   // GLfloat front[3];
   cam_front.x = cos(TWO_PI * th/360.0) * cos(TWO_PI * ph/360.0);
   cam_front.y = sin(TWO_PI * ph/360.0);
   cam_front.z = sin(TWO_PI * th/360.0) * cos(TWO_PI * ph/360.0);
   normalizeVector(&cam_front);
}

double dimmer = 0;

// lol
void logic()
{
   int chunk_x = (int) (floor((cam_pos.x + half_chunk_size) / chunk_size));
   int chunk_z = (int) (floor((cam_pos.z + half_chunk_size) / chunk_size));

   // printf("getting chunk (%d,%d)\n", chunk_x, chunk_z);

   chunk_t * cur_chunk = getChunk(chunk_x, chunk_z, chunk_cache);

   // collisions
   if(cur_chunk)
   {
      int chunk_off_x = ceil((cam_pos.x - half_chunk_size) / chunk_size);
      int chunk_off_z = ceil((cam_pos.z - half_chunk_size) / chunk_size);

      float vert_x = amod(chunk_off_x * chunk_size - cam_pos.x, chunk_size, half_chunk_size);
      float vert_z = amod(chunk_off_z * chunk_size - cam_pos.z, chunk_size, half_chunk_size);

      // maybe reverse this? i did...
      int idx_x = CHUNK_RES - (((vert_x + half_chunk_size) / chunk_size) * CHUNK_RES) - 1;
      int idx_z = CHUNK_RES - (((vert_z + half_chunk_size) / chunk_size) * CHUNK_RES) - 1;


      // printf("x: %f z: %f\n", vert_x, vert_z);
      float nearest_height = (cur_chunk->mesh[(idx_z * (chunk_res_verts)) + idx_x]) * chunk_size;

      // for(int i = 0; i < chunk_res_verts; i++)
      //    for(int j = 0; j < chunk_res_verts; j++)
      //    {
      //       printf("%f\n", cur_chunk->mesh[(i * chunk_res_verts) + j]);
      //    }

      // printf("x: %d z: %d\ty:%f\n", idx_x, idx_z, nearest_height);

      if(cam_pos.y < (float)nearest_height + collision_fudge)
         cam_pos.y = (float)nearest_height + collision_fudge;

      // printf("%f\n", nearest_height);
      // find nearest vert
      // amod();
      // take mod of where we are
      // bias halfway into chunk
   }
}

void display()
{
   processInput();
   logic();

   fogColor[0] = 1-pow(Cos((zh-270)/2.0), 4);
   fogColor[1] = 1-pow(Cos((zh-270)/2.0), 2);//pow(Sin((zh-270)/2.0), 4);
   fogColor[2] = 1-pow(Cos((zh-270)/2.0), 2);

   // dimmer = 1-pow(Cos((zh-270)/2.0), 2);
   glClearColor(fogColor[0], fogColor[1], fogColor[2], 1.0);
   glFogfv(GL_FOG_COLOR, fogColor);
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL


   //  Undo previous transformations
   glLoadIdentity();

   //  Perspective - set eye position
   // double Ex = -2*dim*Sin(th)*Cos(ph);
   // double Ey = +2*dim        *Sin(ph);
   // double Ez = +2*dim*Cos(th)*Cos(ph);
   // gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   // feels silly to do this multiple times?
   gluLookAt(
      // cam_pos.x,cam_pos.y,cam_pos.z,
      0,0,0,
      cam_front.x, cam_front.y, cam_front.z,
      0.0f,1.0f,0.0f
      ); 

   glShadeModel(GL_SMOOTH);

   glDisable(GL_DEPTH_TEST);

      //  Light switch
   if (light)
   {
      //  Translate intensity to color vectors
      float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
      // float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
      float Diffuse[]   = {fogColor[0] * 0.5f, fogColor[1] * 0.5f, fogColor[2] * 0.5f ,1.0};
      float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
      //  Light position
      float pos_sun[]  = {0.0,distance*Sin(zh),distance*Cos(zh),0.0};
      float pos_moon[]  = {0.0,distance*Sin(zh + 180),distance*Cos(zh + 180),0.0};
      //  Draw light pos_sun as ball (still no lighting here)
      glColor3f(1,1,1);

      ball(pos_sun[0],pos_sun[1],pos_sun[2] , 200);
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
      // maybe cap this? some kind of function
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,pos_sun);

      // draw moon 
      // TODO texture
      ball(pos_moon[0],pos_moon[1],pos_moon[2] , 150);
   }
   else
      glDisable(GL_LIGHTING);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_MULTISAMPLE);

   int chunk_off_x = ceil((cam_pos.x - half_chunk_size) / chunk_size);
   int chunk_off_z = ceil((cam_pos.z - half_chunk_size) / chunk_size);

   for (int x_chunk = -render_dist; x_chunk <= render_dist; x_chunk++) {
      for (int z_chunk = -render_dist; z_chunk <= render_dist; z_chunk++) {
         // if((x_chunk * x_chunk) + (z_chunk * z_chunk) > (render_dist * render_dist)) continue;
         
         // int adjusted_x = x_chunk + chunk_off_x;
         // int adjusted_z = z_chunk + chunk_off_z;

         // if ((adjusted_x + adjusted_z) % 2) {
         //       glColor3f(1, 1, 1);
         // } else {
         //       glColor3f(0, 0, 0);
         // }

         int chunk_world_x = (int) (floor((cam_pos.x + half_chunk_size) / chunk_size) + x_chunk);
         int chunk_world_z = (int) (floor((cam_pos.z + half_chunk_size) / chunk_size) + z_chunk);

         // double x_val = sin((chunk_world_x * 5.0) + 1) / 2.0;
         // double z_val = cos((chunk_world_z * 2.5) + 1) / 2.0;

         // if((adjusted_x + adjusted_z) % 2)
         //    glColor3f(1-x_val,1-z_val,1);
         // else
         //    glColor3f(x_val,z_val,0);

         glColor3f(0.245, 0.650, 0.208);

         chunk_t * chunk = getChunk(chunk_world_x, chunk_world_z, chunk_cache);

         if(!chunk)
         {
            chunk = malloc(sizeof(chunk_t));
            initChunk(chunk, chunk_world_x, chunk_world_z);
            generateChunk(chunk);
            cacheChunk(chunk, chunk_cache);
            // printf("generating chunk.\n");
         }

         drawChunk(
               chunk,
               x_chunk * chunk_size + amod(chunk_off_x * chunk_size - cam_pos.x, chunk_size, half_chunk_size),
               -cam_pos.y,
               z_chunk * chunk_size + amod(chunk_off_z * chunk_size - cam_pos.z, chunk_size, half_chunk_size),
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
   float center_x = amod(chunk_off_x * chunk_size - cam_pos.x, chunk_size, half_chunk_size);
   float center_z = amod(chunk_off_z * chunk_size - cam_pos.z, chunk_size, half_chunk_size);
   int it_z[] = {-1,-1,+1,+1};
   int it_x[] = {-1,+1,+1,-1};
   glColor4f(0.0,0.0,1.0,0.5);
   glNormal3f(0.0,1.0,0.0);
   glBegin(GL_QUADS);
   for(int i = 0; i < 4; i++)
   {
      float x = center_x + (it_x[i] * (chunk_size * ((render_dist*2)+1) / 2.0));
      float z = center_z + (it_z[i] * (chunk_size * ((render_dist*2)+1) / 2.0));
      glVertex3f(x, water_level-cam_pos.y, z);
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
      char buff[MAX_PARAM_STR];
      params[i].toStr(&params[i], buff);
      Print("%s%s", (cursor == i ? "->" : "  "), buff);
   }

   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

void idle()
{
   double t = glutGet(GLUT_ELAPSED_TIME) / 100.0;
   zh = fmod(t,210.0)-15;
   // zh = fmod(t,360.0f);
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

      // int dir;
      // inefficient and lame
      case GLUT_KEY_LEFT:
         params[cursor].incr(&params[cursor], -1);
         if(params[cursor].onChange) params[cursor].onChange(&params[cursor]);
      break;
      case GLUT_KEY_RIGHT:
         params[cursor].incr(&params[cursor], 1);
         if(params[cursor].onChange) params[cursor].onChange(&params[cursor]);
      break;
   }

   // TODO rework this...
   configureFog(render_dist);
   // Update projection
   Project(fov,asp,dim);
}

void reshape(int width,int height)
{
   wHeight = height;
   wWidth = width;

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

   glutSetCursor(GLUT_CURSOR_NONE);
   glutPassiveMotionFunc(mouseMove);
   glutIdleFunc(idle);
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   init();
   glutMainLoop();
   return 0;
}