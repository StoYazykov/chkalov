#include "cv.h"

void cv_init(cv *v, size_t c, size_t s) {
    v->d=malloc(c*s);
    v->c=c;
    v->s=0;
    v->es=s;
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
    if(v->s>=v->c) {
        v->d=realloc(v->d, (v->c<<=1)*v->es);
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

// последний элемент. Не POP'ит. Просто возвращает его в *e.
void *cv_back(cv *v) {
    return v->d+(v->s-1)*v->es;
}

// Возвращает, указатель на элемент с индексом i. Осторожно!!! Он изменяемый!
void *cv_eptr(cv *v, size_t i) {
    return (v->d+(i*v->es));
}

// освобождение. Сказать нечего.
void cv_free(cv *v) {
    free(v->d);
    v->s=v->c=v->es=0;
}
