#include <stdlib.h>
#include <semaphore.h>
#include <stdio.h>

#pragma once

#define DSTACK_INIT_SIZE 64
#define DSTACK_SIZE_INCREMENT 64

typedef struct
{
    size_t ct;
    size_t size;

    sem_t items_avail;
    sem_t lock;

    void **bottom;
} dstack;

int dstack_init(dstack *dstack);
int dstack_destroy(dstack *dstack);
int dstack_push(dstack *dstack, void *item);
int dstack_pop(dstack *dstack, void **item);