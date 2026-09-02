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

#ifndef _TOKEN_H
#define _TOKEN_H

#include "../chkalov.h"

typedef enum {
    NOT, ENOF,
    NUMBER, STRING, LCALL, ID, CONVERT,
    VAR, FUN, IF, ELSE, WHILE, RETURN, IMPORT, NATIVE,
    PLUS, MINUS, STAR, SLASH, ASSIGN,
    EQ, NEQ, LT, BT, LE, BE,
    LPAREN, RPAREN, LBRACE, RBRACE,
    COMMA, DOT, COLON,
    CLASS
} TokenType;

typedef struct {
    TokenType type;
    ds value;
} Token;

void tok_init(Token *a, TokenType b, ds c);
void tok_free(Token *a);

#endif
