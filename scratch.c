//    #pragma omp parallel for collapse(2)
//    for(int z = 0; z < chunk_res_verts; z++)
//       for(int x = 0; x < chunk_res_verts+1; x++)
//       {
//          // shared setup
//          int n_x = x-1;

//          float vert_x = adj_x + (x / (double) chunk_res_faces * chunk_size) - half_chunk_size;
//          float vert_z = adj_z + (z / (double) chunk_res_faces * chunk_size) - half_chunk_size;

//          float sbiome = 0.00005f / (BIOME_MAP_WIDTH);

//          // leading logic
//          if(x < chunk_res_verts)
//          {
//             float biome_x = pingPong(fabs(stb_perlin_noise3(vert_x * sbiome, -1, vert_z * sbiome, 0, 0, 0) * (3.0f * BIOME_MAP_WIDTH)), BIOME_MAP_WIDTH);
//             float biome_z = pingPong(fabs(stb_perlin_noise3(vert_x * sbiome, -2, vert_z * sbiome, 0, 0, 0) * (3.0f * BIOME_MAP_WIDTH)), BIOME_MAP_WIDTH);

//             // iterate over all biomes
//             float sum = 0;
//             for(int bm_x = 0; bm_x < BIOME_MAP_WIDTH; bm_x++)
//                for(int bm_z = 0; bm_z < BIOME_MAP_WIDTH; bm_z++)
//                {
//                   // find distance (squared cuz why not SIKE NO LONGER SQUARED WE OVERFLOW)
//                   float val = sqrt(pow(((bm_x+0.5))-(biome_x), 2)+pow(((bm_z+0.5))-(biome_z), 2));

//                   // apply "blend function" that will dictate blendiness
//                   val = (1.0 / pow(val+1, 10));

//                   global_weights[z][x][bm_z][bm_x] = val;
//                   sum += val;
//                }

//             // normalize
//             for(int bm_x = 0; bm_x < BIOME_MAP_WIDTH; bm_x++)
//                for(int bm_z = 0; bm_z < BIOME_MAP_WIDTH; bm_z++)
//                   global_weights[z][x][bm_z][bm_x] /= sum;
            
//             // run each biome generation functions weighted by above
//             float height = 0;
//             for(int bm_x = 0; bm_x < BIOME_MAP_WIDTH; bm_x++)
//                for(int bm_z = 0; bm_z < BIOME_MAP_WIDTH; bm_z++)
//                {
//                   biome_t * cur = biome_map[bm_x][bm_z];
//                   float weight = global_weights[z][x][bm_x][bm_z];

//                   // get landscape height
//                   height += weight * (cur->terrainGen(cur, vert_x, vert_z) / chunk_size);
//                }

//             int idx = (z * (chunk_res_verts)) + x;
//             chunk->mesh[idx] = height;
//          }

//          // following logic
//          if(n_x > 0)
//          {

//          }
//       }
