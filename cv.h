#ifndef _CV_H
#define _CV_H
#include <string.h>
#include <stdlib.h>

typedef struct {
    void *d; // данные.
    size_t s; // размер, ЗАНЯТЫЙ элементами.
    size_t c; // общая ёмкость (в том числе, и запас).
    size_t es; // размер, одного элемента.
} cv;
void cv_init(cv *v, size_t c, size_t s);
void cv_resize(cv *v, size_t s);
void cv_push(cv *v, const void *e);
void cv_pop(cv *v, void *e);
void cv_popr(cv *v);
void *cv_back(cv *v);
void *cv_eptr(cv *v, size_t i);
void cv_free(cv *v);
#endif
