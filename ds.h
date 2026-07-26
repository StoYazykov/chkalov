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
