#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

#include "graphics_utils.h"
#include "CSCIx229.h"
#include "global_config.h"
#include <float.h>

#ifndef _CHUNK_H_
#define _CHUNK_H_

typedef struct bundle
{
   float mesh;
   vtx normal;
   color_t color;
} bundle;

typedef struct chunk
{
   int id_x, id_z;
   bundle * bundles;
   // oh boy
   // "cpu side" data, will be buffered into GPU at end of chunk generation
   GLuint vbo_id;
   // GLfloat * vboData;
   // TODO
   // clutter stuff
} chunk_t;

typedef struct biome
{
   float (*terrainGen)(struct biome *, double, double);
   // prolly take height as a parameter here, we've got it
   color_t (*colorGen)(struct biome *, float);
} biome_t;










// biome definitions
color_t testColor1(struct biome * self, float height)
{
   color_t ret = { .r = 0.245f, .g = 0.650f, .b = 0.208f};
   return ret;
}
color_t testColor2(struct biome * self, float height)
{
   // color_t ret = { .r = 0.245f, .g = 0.650f, .b = 0.208f};
   color_t ret = { .r = (sin(height / 100.0)+1)/2.0, .g = 1.0f, .b = 1.0f};
   
   return ret;
}
color_t testColor3(struct biome * self, float height)
{
   return (color_t) { .r = 0.850f, .g = 0.761f, .b = 0.365f};
}

float testTerrain1(struct biome * self, double x, double z)
{
   return 75.0f;
}
float testTerrain2(struct biome * self, double x, double z)
{
   return 500 * sin(x / 100.0);
}
float testTerrain3(struct biome * self, double x, double z)
{
   return 500 * sin(z / 100.0);
}
float testTerrain4(struct biome * self, double x, double z)
{
   return 500 * (sin(x / 1000.0)+sin(z / 1000.0));
}
float testTerrain5(struct biome * self, double x, double z)
{
   return 500 * pow(sin(x / 1000.0)+sin(z / 1000.0),2);
}
float testTerrain6(struct biome * self, double x, double z)
{
   return 500 * (sin(sqrt(pow(sin(x / 100.0), 2) + pow(sin(z / 100.0), 2))));
}
float testTerrain7(struct biome * self, double x, double z)
{
   return 50.0f;
}
float testTerrain8(struct biome * self, double x, double z)
{
   return 500.0;
}
float testTerrain9(struct biome * self, double x, double z)
{
   return -500.0;
}











// BEGIN DUNES
float dunesTerrain(struct biome * self, double x, double z)
{
   float s = 0.001f;

   float height = 0.0f;
   height += 250 - (pow(stb_perlin_noise3(x * s, 0, z * s, 10, 0, 0), 1) * 250.0);
   height += stb_perlin_ridge_noise3(x * s, 0, z * s, 2.0, 0.5, 0.5, 1) * 250;

   return height;
}

color_t dunesColor(struct biome * self, float height)
{
   return (color_t) { .r = 0.850f, .g = 0.761f, .b = 0.365f};
}

biome_t dunes_biome = {.terrainGen=dunesTerrain, .colorGen=dunesColor };
// END DUNES

// BEGIN MESA
float mesaTerrain(struct biome * self, double x, double z)
{
   float s1 = 0.001f;
   float s2 = 0.00025f;
   float peak = 500.0f;

   float height = 0.0f;
   // height += pow(stb_perlin_ridge_noise3(x * s, 0, z * s, 2.0, 0.5, 0.5, 1) * 10, 10.0) * -5000.0f;

   // mesa cut-outs
   // output from stf func is 0-0.5, but that depends on params.
   // height += peak - (pow(stb_perlin_ridge_noise3(x * s1, 0, z * s1, 2.0, 0.5, 1.0, 1) * 2, 15) * peak);

   height += stb_perlin_noise3(x * s2, 0, z * s2, 0, 0, 0) * 2000.0f;
   // TODO ICKY MAGIC NUMBER
   height += 500.0f;
   // lock to flat tops
   height = roundTo(height, 500.0f)-100.0f;
   // some noise
   height += stb_perlin_noise3(x * s1, 0, z * s1, 0, 0, 0) * 100.0f;

   return height;
}

color_t mesaColor(struct biome * self, float height)
{
   return (color_t) { .r = 0.850f, .g = 0.761f, .b = 0.365f};
}

biome_t mesa_biome = {.terrainGen=mesaTerrain, .colorGen=mesaColor };
// END MESA

// BEGIN OG
float ogTerrain(struct biome * self, double x, double z)
{
   float s2 = 0.004f;
   float s3 = 0.002f;
   float s4 = 0.0005f;

   float base = (stb_perlin_noise3(x * s4, 0, z * s4, 0, 0, 0) + 1) / 2.0;
   float hills = base * 1000;
   float mountains = smoothstep(.5,.95,base) * pow((stb_perlin_noise3(x * s2, 1, z * s2, 0, 0, 0) + 1) / 2.0 * 2, 3) * 100;
   float lakes = smoothstep(0.5,0.95,1.0-base) * (stb_perlin_noise3(x * s3, 2, z * s3, 0, 0, 0) - 1) / 2.0 * 500;
   
   float height = 0;
   height += hills;
   height += mountains;
   height += lakes;
   // TODO MAGIC NUMBER
   height += -250.0f;
   // height += cam_y_offset;

   return height;
}

color_t ogColor(struct biome * self, float height)
{
   return (color_t) { .r = 0.245f, .g = 0.650f, .b = 0.208f};
}

biome_t og_biome = {.terrainGen=ogTerrain, .colorGen=ogColor };
// END OG

// BEGIN OCEAN
float oceanTerrain(struct biome * self, double x, double z)
{
   return -100.0f;
}

color_t oceanColor(struct biome * self, float height)
{
   return (color_t) { .r = 0.850f, .g = 0.761f, .b = 0.365f };
}

biome_t ocean_biome = {.terrainGen=oceanTerrain, .colorGen=oceanColor };
// END OCEAN

// BEGIN ICEBERGS
float icebergTerrain(struct biome * self, double x, double z)
{
   float s1 = 0.00025f;
   float s2 = 0.00025f;
   float shelf = 100.0f;

   float height = 0.0f;
   // height += pow(stb_perlin_ridge_noise3(x * s, 0, z * s, 2.0, 0.5, 0.5, 1) * 10, 10.0) * -5000.0f;

   // iceberg cut-outs
   // output from stf func is 0-0.5, but that depends on params.
   height += shelf - (pow(stb_perlin_ridge_noise3(x * s1, 0, z * s1, 2.0, 0.5, 1.0, 1) * 2, 20) * 5 * shelf); 

   height = smoothMax(height, -100.0f, 10.0f);

   return height;
}

color_t icebergColor(struct biome * self, float height)
{
   return (color_t) { .r = 1.0f, .g = 1.0f, .b = 1.0f};
}

biome_t iceberg_biome = {.terrainGen=icebergTerrain, .colorGen=icebergColor };
// END ICEBERGS





// BEGIN ICEMTSMALL
float iceMtSmallTerrain(struct biome * self, double x, double z)
{
   float s1 = 0.0005f;
   float s2 = 0.0001f;

   float snow_drift = 0;
   snow_drift += 250 - (pow(stb_perlin_noise3(x * s1, 0, z * s1, 10, 0, 0), 1) * 250.0);
   snow_drift += stb_perlin_ridge_noise3(x * s1, 0, z * s1, 2.0, 0.5, 0.5, 1) * 250;

   float height = 0.0f;
   height += pow(stb_perlin_fbm_noise3(x*s2, 0, z*s2, 2.0, 0.5, 6)+1, 3) * 500.0f;// + 100.0f;
   height = smoothMax(height, ((stb_perlin_noise3(x*s2, 0, z*s2, 0, 0, 0) + 1) * 250) + snow_drift, 250.0f);
   // height = fmax(height, (stb_perlin_noise3(x*s2, 0, z*s2, 0, 0, 0) + 1) * 500);
   return height;
}

color_t iceMtSmallColor(struct biome * self, float height)
{
   return (color_t) { .r = 1.0f, .g = 1.0f, .b = 1.0f};
}

biome_t iceMtSmall_biome = {.terrainGen=iceMtSmallTerrain, .colorGen=iceMtSmallColor };
// END ICEMTSMALL

// BEGIN TEST
float testTerrain(struct biome * self, double x, double z)
{
   float height = 0.0f;

   return height;
}

color_t testColor(struct biome * self, float height)
{
   return (color_t) { .r = 1.0f, .g = 1.0f, .b = 1.0f};
}

biome_t test_biome = {.terrainGen=testTerrain, .colorGen=testColor };
// END TEST



biome_t b1 = { .terrainGen=testTerrain1, .colorGen=testColor1 };
biome_t b2 = { .terrainGen=testTerrain2, .colorGen=testColor1 };
biome_t b3 = { .terrainGen=testTerrain3, .colorGen=testColor1 };
biome_t b4 = { .terrainGen=testTerrain4, .colorGen=testColor2 };
biome_t b5 = { .terrainGen=testTerrain5, .colorGen=testColor2 };
biome_t b6 = { .terrainGen=testTerrain6, .colorGen=testColor2 };
biome_t b7 = { .terrainGen=testTerrain7, .colorGen=testColor3 };
biome_t b8 = { .terrainGen=testTerrain8, .colorGen=testColor3 };
biome_t b9 = { .terrainGen=testTerrain9, .colorGen=testColor3 };

#define BIOME_MAP_WIDTH 4
biome_t * biome_map[BIOME_MAP_WIDTH][BIOME_MAP_WIDTH] = 
// {
//    {&b1, &b2, &b3},
//    {&b4, &b5, &b6},
//    {&b7, &b8, &b9}
// };
{
   { &mesa_biome, &dunes_biome,  &ocean_biome,     &og_biome },
   { &mesa_biome, &ocean_biome,  &og_biome,        &og_biome },
   { &dunes_biome,&iceberg_biome,&iceberg_biome,   &iceMtSmall_biome },
   { &og_biome,   &iceberg_biome,&iceMtSmall_biome,&iceMtSmall_biome }
};
// {
//    {&ocean_biome, &dunes_biome, &mesa_biome},
//    {&b1, &og_biome, &iceberg_biome},
//    {&b7, &iceberg_biome, &iceMtSmall_biome}
// };
// {
//    {&iceberg_biome}
// };

inline void initChunk(chunk_t * chunk, int id_x, int id_z)
{
   size_t memsize;
   chunk->id_x = id_x;
   chunk->id_z = id_z;

   memsize = sizeof(bundle) * chunk_res_verts * chunk_res_verts;
   chunk->bundles = malloc(memsize);
   memset(chunk->bundles, 0, memsize);

   // all GLfloats, 3 for pos, 3 for norm, 3 for color, entry for all verticies...
   // memsize = sizeof(GLfloat) * (3 + 3 + 3) * 2 * chunk_res_verts * (chunk_res_verts-1);
   // chunk->vboData = malloc(memsize);
   // memset(chunk->vboData, 0, memsize);
}


inline void destroyChunk(chunk_t * chunk)
{
   glDeleteBuffers(1, &(chunk->vbo_id));

   free(chunk->bundles);
   // free(chunk->vboData);
   free(chunk);
}

inline void flushChunkCache(chunk_t *chunk_cache[CHUNK_CACHE_SIZE][CHUNK_CACHE_SIZE])
{
   for(int i = 0; i < CHUNK_CACHE_SIZE; i++)
      for(int j = 0; j < CHUNK_CACHE_SIZE; j++)
         if(chunk_cache[i][j])
            destroyChunk(chunk_cache[i][j]);

   memset(chunk_cache, 0, sizeof(chunk_t *) * CHUNK_CACHE_SIZE * CHUNK_CACHE_SIZE);
}

inline void cacheChunk(chunk_t * chunk, chunk_t *chunk_cache[CHUNK_CACHE_SIZE][CHUNK_CACHE_SIZE]) 
{
    chunk_t **target = &chunk_cache[imod(chunk->id_z, CHUNK_CACHE_SIZE)][imod(chunk->id_x, CHUNK_CACHE_SIZE)];

    if(*target) destroyChunk(*target);

    *target = chunk;
}

inline chunk_t * getChunk(int id_x, int id_z, chunk_t *chunk_cache[CHUNK_CACHE_SIZE][CHUNK_CACHE_SIZE])
{
   chunk_t * fetched = chunk_cache[imod(id_z, CHUNK_CACHE_SIZE)][imod(id_x, CHUNK_CACHE_SIZE)];

   if(fetched && !(fetched->id_x == id_x && fetched->id_z == id_z))
      return (chunk_t *) NULL;   

   return fetched;
}

//TODO TEMP
// float min_val = 10;
// float max_val = 0;

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

         float sbiome = 0.00005f / (BIOME_MAP_WIDTH);
         float s_altitude = 0.00001f;

         float altitude = stb_perlin_noise3(vert_x * s_altitude, 0, vert_z * s_altitude, 0, 0, 0) * 10000.0;
         
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
         // MOST RECENT
         // float base = (stb_perlin_noise3(vert_x * s4, 0, vert_z * s4, 0, 0, 0) + 1) / 2.0;
         // float hills = base * 1000;
         // float mountains = smoothstep(.5,.95,base) * pow((stb_perlin_noise3(vert_x * s2, 1, vert_z * s2, 0, 0, 0) + 1) / 2.0 * 2, 3) * 100;
         // float lakes = smoothstep(0.5,0.95,1.0-base) * (stb_perlin_noise3(vert_x * s3, 2, vert_z * s3, 0, 0, 0) - 1) / 2.0 * 500;
         
         // chunk->bundles[(z * (chunk_res_verts)) + x].mesh = 
         //     hills + 
         //     mountains + 
         //     lakes +

         //     cam_y_offset
         // ;

         // translate x, z to [0-1], [0-1] via noise map to select target within biome map
         // BELOW IS HEAVILY BIASED TOWARDS CENTER OF BIOME MAP
         // float biome_x = ((stb_perlin_noise3(vert_x * sbiome, -1, vert_z * sbiome, 0, 0, 0) + 1) / 2.0) * BIOME_MAP_WIDTH;
         // float biome_z = ((stb_perlin_noise3(vert_x * sbiome, -2, vert_z * sbiome, 0, 0, 0) + 1) / 2.0) * BIOME_MAP_WIDTH;


         // TODO this is really trash. this is my current solution for mapping noise to fixed range without skew
         float biome_x = pingPong(fabs(stb_perlin_noise3(vert_x * sbiome, -1, vert_z * sbiome, 0, 0, 0) * (2.0f * BIOME_MAP_WIDTH)), BIOME_MAP_WIDTH);
         float biome_z = pingPong(fabs(stb_perlin_noise3(vert_x * sbiome, -2, vert_z * sbiome, 0, 0, 0) * (2.0f * BIOME_MAP_WIDTH)), BIOME_MAP_WIDTH);

         // if(biome_x < min_val || biome_x > max_val)
         // {
         //    min_val = fmin(min_val, biome_x);
         //    max_val = fmax(max_val, biome_x);
         //    printf("min: %f max: %f\n", min_val, max_val);
         // }

         

         // iterate over all biomes
         float sum = 0;
         float weights[BIOME_MAP_WIDTH][BIOME_MAP_WIDTH];
         for(int bm_x = 0; bm_x < BIOME_MAP_WIDTH; bm_x++)
            for(int bm_z = 0; bm_z < BIOME_MAP_WIDTH; bm_z++)
            {
               // find distance (squared cuz why not SIKE NO LONGER SQUARED WE OVERFLOW)
               float val = sqrt(pow(((bm_x+0.5))-(biome_x), 2)+pow(((bm_z+0.5))-(biome_z), 2));

               // apply "blend function" that will dictate blendiness
               val = (1.0 / pow(val+1, 10));

               weights[bm_z][bm_x] = val;
               sum += val;
            }

         // normalize
         for(int bm_x = 0; bm_x < BIOME_MAP_WIDTH; bm_x++)
            for(int bm_z = 0; bm_z < BIOME_MAP_WIDTH; bm_z++)
               weights[bm_z][bm_x] /= sum;

         // TODO if contribution is under threshold, dont generate to save cpu
         // will have to renormalize to avoid sinking

         // we have the weights!
         // run each biome generation functions weighted by above
         float height = 0;
         color_t primary = {.r=0, .g=0, .b=0 };
         color_t secondary;
         for(int bm_x = 0; bm_x < BIOME_MAP_WIDTH; bm_x++)
            for(int bm_z = 0; bm_z < BIOME_MAP_WIDTH; bm_z++)
            {
               biome_t * cur = biome_map[bm_x][bm_z];
               float weight = weights[bm_x][bm_z];
               // get landscape height
               height += weight * cur->terrainGen(cur, vert_x, vert_z);

               // get landscape color (c makes this really cringe)
               secondary = cur->colorGen(cur, height);
               color_t multiplier = {.r=weight, .g=weight, .b=weight };
               primary = colorAdd(primary, colorMult(secondary, multiplier));
            }

         bundle * bund = &chunk->bundles[(z * (chunk_res_verts)) + x];

         bund->mesh = height;
         bund->color = primary;

      }

   // generate norms on per-face basis
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

   // assemble vbo data!
   /*
   A--C--E
   |  |  |
   B--D--F
   */
   //   GLfloat * vdat = chunk->vboData;
   GLfloat vdat[(3 + 3 + 3) * 2 * chunk_res_verts * (chunk_res_verts-1)];
   bundle * bdls = chunk->bundles;
   int it = 0;
   int dat_size = (3 + 3 + 3);
   for(int z = 0; z < chunk_res_verts-1; z++)
      for(int x = 0; x < chunk_res_verts; x++)
      {
         // TODO: make loop
         bundle * bdl1 = &bdls[(z * (chunk_res_verts)) + x];
         float x1 = (frag*x)-0.5;
         float y1 = bdl1->mesh;
         float z1 = (frag*z)-0.5;

         bundle * bdl2 = &bdls[((z+1) * (chunk_res_verts)) + x];
         float x2 = x1;
         float y2 = bdl2->mesh;
         float z2 = (frag*(z+1))-0.5;

         // considering vertices in pairs
         int cursor = dat_size * 2 * it;

         // bundle A
         // geom
         vdat[cursor+0] = x1;
         vdat[cursor+1] = y1;
         vdat[cursor+2] = z1;
         // norm
         vdat[cursor+3] = bdl1->normal.x;
         vdat[cursor+4] = bdl1->normal.y;
         vdat[cursor+5] = bdl1->normal.z;
         // color
         // vdat[cursor+6] = 0.245f;
         // vdat[cursor+7] = 0.650f;
         // vdat[cursor+8] = 0.208f;
         vdat[cursor+6] = bdl1->color.r;
         vdat[cursor+7] = bdl1->color.g;
         vdat[cursor+8] = bdl1->color.b;

         cursor += dat_size;

         // bundle B
         vdat[cursor+0] = x2;
         vdat[cursor+1] = y2;
         vdat[cursor+2] = z2;
         // norm
         vdat[cursor+3] = bdl2->normal.x;
         vdat[cursor+4] = bdl2->normal.y;
         vdat[cursor+5] = bdl2->normal.z;
         // color
         vdat[cursor+6] = bdl2->color.r;
         vdat[cursor+7] = bdl2->color.g;
         vdat[cursor+8] = bdl2->color.b;

         it++;
      }

   glGenBuffers(1, &(chunk->vbo_id));
   glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_id);
   glBufferData(
      GL_ARRAY_BUFFER, 
      (sizeof(GLfloat) * (3 + 3 + 3) * 2 * chunk_res_verts * (chunk_res_verts-1)), 
      vdat, 
      GL_STATIC_DRAW
      );
   glBindBuffer(GL_ARRAY_BUFFER, 0);   
}

inline void drawChunkCPU(chunk_t * chunk, double screen_x, double y, double screen_z, int id_x, int id_z)
{
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

   // Save transformation
   glPushMatrix();

   double frag = 1.0 / chunk_res_faces;

   glTranslated(screen_x,y,screen_z);
   glScaled(chunk_size, 1.0, chunk_size);
   
   for(int z = 0; z < chunk_res_verts-1; z++)
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
      }
      glEnd();
   }
   
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

   double frag = 1.0 / chunk_res_faces;

   glTranslated(screen_x,y,screen_z);
   glScaled(chunk_size, 1.0, chunk_size);

   const GLsizei stride = sizeof(GLfloat) * 9; // 3 for position, 3 for normal, 3 for color
   const GLvoid* vertexOffset = (const GLvoid*)(sizeof(GLfloat) * 0); // offset at the beginning
   const GLvoid* normalOffset = (const GLvoid*)(sizeof(GLfloat) * 3); // offset after 3 floats of vertex
   const GLvoid* colorOffset = (const GLvoid*)(sizeof(GLfloat) * 6); // offset after 3 floats of vertex and 3 of normal
   
   glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_id);

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_NORMAL_ARRAY);
   glEnableClientState(GL_COLOR_ARRAY);

   glVertexPointer(3, GL_FLOAT, stride, vertexOffset);
   glNormalPointer(GL_FLOAT, stride, normalOffset);
   glColorPointer(3, GL_FLOAT, stride, colorOffset);   

   for(int strip = 0; strip < chunk_res_faces; strip++)
   {
      int verts_per_strip = 2 * chunk_res_verts;
      glDrawArrays(
         GL_QUAD_STRIP, 
         strip * verts_per_strip, 
         verts_per_strip
         );
   }

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   
   glPopMatrix();
}

#endif