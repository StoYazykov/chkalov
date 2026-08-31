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

#ifndef _CV_H
#define _CV_H
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

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
bool cv_equ(cv *a, cv *b);
void cv_free(cv *v);
#endif
