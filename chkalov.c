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

#include "chkalov.h"

uint8_t selszu(uint64_t a) {
    if(!a) return 0;
    if(a<256) return 0x01;
    if(a<65536) return 0x02;
    if(a<(1ULL<<32)) return 0x04;
    return 0x08;
}

uint8_t selsz(int64_t a) {
    if(!a) return 0;
    if(a>-128&&a<128) return 0x01;
    if(a>-32768&&a<32768) return 0x02;
    if(a>-(1LL<<31)&&a<(1LL<<31)) return 0x04;
    return 0x08;
}
