#include <math.h>
#include <omp.h>

#include "CSCIx229.h"
#include "graphics_utils.h"
#include "global_config.h"
#include "chunk.h"
#include "parameter.h"
#include "dstack.h"

int main()
{
    printf("test start\n");
    // chunk_t chunk;
    // for(int i = 0; i < 100; i++)
    //     generateChunk(&chunk);
    
    dstack ds;
    int *hold;

    dstack_init(&ds);

    dstack_pop(&ds, (void **) &hold);

    printf("test end\n");
    exit(0);
}