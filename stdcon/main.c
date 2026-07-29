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

#include "../chkapi.h"

Slot print(size_t argc, Slot *argp, ChkEnv *env) {
    if(ISSTR(argp->type)) printf("%s", *env->_heap+argp->value);
    else printf("%d", argp->value);
    fflush(stdout);
    return (Slot){0x00};
}

Slot println(size_t argc, Slot *argp, ChkEnv *env) {
    printf("%d\r\n", argp->value);
    return (Slot){0x00};
}

Slot readln(size_t argc, Slot *argp, ChkEnv *env) {
    print(argc, argp, env);
    char *mem=env->alloc(env, 256);
    fgets(mem, 256, stdin);
    return (Slot){STR, mem-(uintptr_t)*env->_heap};
}

Slot readint(size_t argc, Slot *argp, ChkEnv *env) {
    int a;
    print(argc, argp, env);
    scanf("%d", &a);
    return (Slot){INT, a};
}
