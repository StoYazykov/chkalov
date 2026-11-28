/*
 * Copyright (C) 2025-2026  StoYazykov
 *
 * This file is part of Chkalov.
 *
 * Chkalov is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Chkalov is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Chkalov.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "cv.h"

void cv_init(cv *v, size_t c, size_t s) {
    v->d=malloc(c*s);
    v->c=c;
    v->s=0;
    v->es=s;
    //fprintf(stderr, "CV_INIT: c=%lx, es=%lx\r\n", c, s);
}

void cv_resize(cv *v, size_t s) {
    if(s>v->c) {
        v->d=realloc(v->d, s*v->es<<1);
        v->c=s<<1;
    }
    v->s=s;
}

// Пуш. Сигнатура cv_push(вектор, указатель_на_элемент)
void cv_push(cv *v, const void *e) {
   // printf("cv_push, capacity %zu bytes, size %zu bytes\n", (v->c*v->es), v->s);
    if(v->s>=v->c) {
        void *t=v->d;
        v->d=realloc(t, (v->c<<=1)*v->es);
        if(!v->d) {
            free(t);
            error("Out of memory, requested %f KB", (v->c*v->es)/1024);
        }
    }
    memcpy(v->d+(v->es*v->s), e, v->es);
    v->s++;
}

// Поп с возвратом значения. Сигнатура cv_pop(вектор, указатель_куда_будем_класть_значение)
void cv_pop(cv *v, void *e) {
    memcpy(e, v->d+(v->s-1)*v->es, v->es);
    v->s--;
}

// Поп с потерей значения. Сигнатура cv_popr(вектор)
void cv_popr(cv *v) {
    v->s--;
}

void cv_copy(cv *t, cv *f) {
    t->c=f->c, t->es=f->es, t->s=f->s;
    t->d=malloc(t->c*t->es);
    memcpy(t->d, f->d, t->s*t->es);
}

// последний элемент. Не POP'ит. Просто возвращает его.
void *cv_back(cv *v) {
    return v->d+(v->s-1)*v->es;
}

// Возвращает, указатель на элемент с индексом i. Осторожно!!! Он изменяемый!
void *cv_eptr(cv *v, size_t i) {
    return (v->d+(i*v->es));
}

bool cv_equ(cv *a, cv *b) {
    /*bool memres=!memcmp(a->d, b->d, a->s*a->es);
    char *pa, *pb, *e;
    e=((char *)a->d)+a->s*a->es;
    pa=a->d;
    pb=b->d;
    while(pa<e) {
        fprintf(stderr, "a 0x%.2x\nb 0x%.2x\n", *pa, *pb);
        pa++, pb++;
    }
    fprintf(stderr, "CV_EQU: a.s=%zu b.s=%zu a.es=%zu b.es=%zu, memory %s\n", a->s, b->s, a->es, b->es, memres?"equal":"not equal");*/
    return (a->s==b->s&&a->es==b->es&&!memcmp(a->d, b->d, a->s*a->es));
}

// освобождение. Сказать нечего.
void cv_free(cv *v) {
    free(v->d);
    v->s=v->c=v->es=0;
}
