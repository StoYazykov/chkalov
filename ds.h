#ifndef _DS_H
#define _DS_H
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

typedef char* ds;

void ds_str(ds *a, const char *b);
void ds_resize(ds *a, size_t ab);
void ds_sub(ds *a, const char *s, size_t n, size_t l);
ds ds_gl(ds *a, FILE *fp);
void ds_cat(ds *a, ...);
void ds_free(ds *a);

#endif
