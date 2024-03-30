#include <math.h>

#include "global_config.h"
#include "CSCIx229.h"

#ifndef _GRAPHICS_UTILS_H_
#define _GRAPHICS_UTILS_H_

typedef struct vtx
{
   float x,y,z;
} vtx;

vtx VERTICAL = { 0, 1, 0 };

typedef struct color
{
   float r,g,b;
} color_t;

// returns its parameter too, cuz why not lmao
// inline color_t * colorMult(color_t * out, color_t * other)
// {
//    out->r *= other->r;
//    out->g *= other->g;
//    out->b *= other->b;

//    return out;
// }

// inline color_t * colorAdd(color_t * out, color_t * other)
// {
//    out->r += other->r;
//    out->g += other->g;
//    out->b += other->b;

//    return out;
// }

inline float pingPong(float value, float max) 
{
   float period = 2 * max;
   float modValue = fmod(value, period);

   if (modValue <= max)
      return modValue;
   else
      return period - modValue;
}

inline float smoothMin(float a, float b, float k)
{
   float h = fmax(k-fabs(a-b), 0) / k;
   return fmin(a, b) - h*h*h*k*1/6.0;
}

inline float smoothMax(float a, float b, float k)
{
   float h = fmax(k-fabs(a-b), 0) / k;
   return fmax(a, b) + h*h*h*k*1/6.0;
}

inline color_t colorMult(color_t a, color_t b)
{
   a.r *= b.r;
   a.g *= b.g;
   a.b *= b.b;

   return a;
}

inline color_t colorAdd(color_t a, color_t b)
{
   a.r += b.r;
   a.g += b.g;
   a.b += b.b;

   return a;
}

inline float roundTo(float number, float x) 
{
   return round(number / x) * x;
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

float lerp(float a, float b, float t)
{
   return (1.0f-t)*a + b*t;
}

inline color_t colorLerp(color_t a, color_t b, float t)
{
   return (color_t) { ((1.0f-t)*a.r + b.r*t), ((1.0f-t)*a.g + b.g*t), ((1.0f-t)*a.b + b.b*t) };
}

inline float smoothstep(float e0, float e1, float x)
{
    float t = fminf(fmaxf((x - e0) / (e1 - e0), 0.0f), 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

inline color_t colorSmoothstep(float e0, float e1, float x, color_t a, color_t b)
{
    float t = fminf(fmaxf((x - e0) / (e1 - e0), 0.0f), 1.0f);
    return colorLerp(a, b, (t * t * (3.0f - 2.0f * t)));
}

inline float dot(vtx a, vtx b)
{
   return a.x * b.x + a.y * b.y + a.z * b.z;
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