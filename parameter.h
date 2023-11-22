#ifndef _PARAMETER_H_
#define _PARAMETER_H_

#include <stdio.h>
#include <stdlib.h>

#define MAX_PARAM_STR 64
#define DEFINE_FUNCS(type, field, print_flag) \
void type##Incr(param_t *self) { \
    type * val = (type *)self->val; \
    *val += self->delta.field; \
    if (*val > self->max.field) \
        *val = self->max.field; \
} \
void type##Decr(param_t *self) { \
    type * val = (type *)self->val; \
    *val -= self->delta.field; \
    if (*val < self->min.field) \
        *val = self->min.field; \
} \
void type##ToStr(param_t *self, char * str) { \
    snprintf(str, MAX_PARAM_STR, "%s: %"#print_flag, self->name, *(type *)self->val); \
}

enum param_type { BOOL, INT, LONG, FLOAT, DOUBLE };

typedef struct param
{
    char name[32];

    enum param_type type;

    void * val;
    union { int i; long l; float f; double d; } delta, min, max;

    void (*incr)(struct param *);
    void (*decr)(struct param *);
    void (*toStr)(struct param *, char *);
} param_t;

// simple cases
DEFINE_FUNCS(int, i, d);
DEFINE_FUNCS(long, l, ld);
DEFINE_FUNCS(float, f, f);
DEFINE_FUNCS(double, d, lf);

// special cases
void boolIncr(param_t * self) { *(int *)self->val = 1; }
void boolDecr(param_t * self) { *(int *)self->val = 0; }
void boolToStr(param_t * self, char * str) { snprintf(str, MAX_PARAM_STR, "%s: %s", self->name, (*(int *)self->val ? "true" : "false")); }

#endif