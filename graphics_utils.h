#include <math.h>

#include "global_config.h"
#include "CSCIx229.h"

#ifndef _GRAPHICS_UTILS_H_
#define _GRAPHICS_UTILS_H_

typedef struct vtx
{
   float x,y,z;
} vtx;

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

#endif