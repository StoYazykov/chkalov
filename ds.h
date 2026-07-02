#ifndef _DS_H
#define _DS_H
#include <string.h>
#include <stdlib.h>

typedef char *ds;

void ds_str(ds *a, const char *b) {
    *a=realloc(*a, strlen(b)+1);
    strcpy(*a, b);
}

void ds_resize(ds *a, size_t ab) {
    *a=realloc(*a, ab+1);
    (*a)[ab]=0;
}

void ds_cat(ds *a, ...) {
    size_t tl=strlen(*a);
    va_list va;
    char *ns;
    va_start(va, a);
    const char *s;
    while((s=va_arg(va, const char *))!=NULL) {
        tl+=strlen(s);
    }
    va_end(va);
    ns=malloc(tl+1);
    strcpy(ns, *a);
    va_start(va, a);
    while((s=va_arg(va, const char *))!=NULL) {
        strcat(ns, s);
    }
    va_end(va);
    free(*a);
    *a=ns;
}

#endif
