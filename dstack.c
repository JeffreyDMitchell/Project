#include "dstack.h"

int dstack_init(dstack *dstack)
{
    sem_init(&dstack->items_avail, 0, 0);
    sem_init(&dstack->lock, 0, 1);

    dstack->bottom = malloc(DSTACK_INIT_SIZE * sizeof(void *));
    dstack->size = DSTACK_INIT_SIZE;
    dstack->ct = 0;

    return 0;
}

int dstack_destroy(dstack *dstack)
{
    sem_destroy(&dstack->items_avail);
    sem_destroy(&dstack->lock);

    free(dstack->bottom);

    return 0;
}

int dstack_push(dstack *dstack, void *item)
{
    sem_wait(&dstack->lock);
    
    if(dstack->ct >= dstack->size)
    {
        dstack->size += DSTACK_SIZE_INCREMENT;
        if(!realloc(dstack->bottom, dstack->size * sizeof(void *)))
        {
            perror("Failed allocation.\n");
            sem_post(&dstack->lock);
            return -1;
        }
    }

    dstack->bottom[dstack->ct++] = item;

    sem_post(&dstack->lock);
    sem_post(&dstack->items_avail);

    return 0;
}

int dstack_pop(dstack *dstack, void **item)
{
    // block until item
    sem_wait(&dstack->items_avail);
    sem_wait(&dstack->lock);

    *item = dstack->bottom[--dstack->ct];

    sem_post(&dstack->lock);

    return 0;
}