#include "ds.h"

void ds_str(ds *a, const char *b) {
    *a=realloc(*a, strlen(b)+1);
    strcpy(*a, b);
}

void ds_resize(ds *a, size_t ab) {
    *a=realloc(*a, ab+1);
    (*a)[ab]=0;
}

void ds_sub(ds *a, const char *s, size_t n, size_t l) {
    *a=realloc(*a, l+1);
    strncpy(*a, s+n, l);
    (*a)[l]=0;
}

ds ds_gl(ds *a, FILE *fp) {
    size_t l=0, cap;
    *a=malloc(cap=128);
    while(true) {
        (*a)[l]=fgetc(fp);
        if(feof(fp)) break;
        l++;
        if(l>=cap) *a=realloc(*a, cap*=2);
    }
    if(!l) {
        free(*a);
        return NULL;
    }
    (*a)[l]=0;
    return *a;
}

void ds_cat(ds *a, ...) {
    size_t tl=strlen(*a);
    va_list va;
    char *ns;
    const char *s;
    va_start(va, a);
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
