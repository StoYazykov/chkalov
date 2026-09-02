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

#include "lexer.h"
#include "parser.h"

void lexer(void *b, ds l) {
    Parser *a=(Parser *)b;
    Token t;
    uint64_t pos=0;
    while(pos<strlen(l)) {
        t.value=NULL;
        while(pos<strlen(l)&&isspace(l[pos])) pos++;

        if(pos>=strlen(l)) {
            Token abcv={EOF, strdup("")};
            cv_push(&a->file, &abcv);
        }
        IFCS(l+pos, "native", 6) {
            pos+=6;
            t=(Token){NATIVE, strdup("native")};
            cv_push(&a->file, &t);
            continue;
        }
        IFCS(l+pos, "class", 5) {
            pos+=5;
            t=(Token){CLASS, strdup("class")};
            cv_push(&a->file, &t);
            continue;
        }
        IFCS(l+pos, "var", 3) {
            pos+=3;
            t=(Token){VAR, strdup("var")};
            cv_push(&a->file, &t);
            continue;
        }
        IFCS(l+pos, "==", 2) {
            pos+=2;
            t=(Token){EQ, strdup("==")};
            cv_push(&a->file, &t);
            continue;
        }
        IFCS(l+pos, "!=", 2) {
            pos+=2;
            t=(Token){NEQ, strdup("!=")};
            cv_push(&a->file, &t);
            continue;
        }
        IFCS(l+pos, ">=", 2) {
            pos+=2;
            t=(Token){BE, strdup(">=")};
            cv_push(&a->file, &t);
            continue;
        }
        IFCS(l+pos, "<=", 2) {
            pos+=2;
            t=(Token){LE, strdup("<=")};
            cv_push(&a->file, &t);
            continue;
        }
        IFCS(l+pos, "if", 2) {
            pos+=2;
            t=(Token){IF, strdup("if")};
            cv_push(&a->file, &t);
            continue;
        }
        IFCS(l+pos, "while", 5) {
            pos+=5;
            t=(Token){WHILE, strdup("while")};
            cv_push(&a->file, &t);
            continue;
        }
        if(isalpha(l[pos])) { // ID
            size_t start=pos;
            while(pos<strlen(l)&&isalnum(l[pos])) pos++;
            t.type=ID;
            t.value=NULL;
            ds_sub(&t.value, l, start, pos-start);
            cv_push(&a->file, &t);
            continue;
        }
        if(isdigit(l[pos])) {
            size_t start=pos;
            while(pos<strlen(l)&&isdigit(l[pos])) pos++;
            t.type=NUMBER;
            t.value=NULL;
            ds_sub(&t.value, l, start, pos-start);
            cv_push(&a->file, &t);
            continue;
        }
        if(l[pos]=='\"') {
            size_t start=++pos;
            while(pos<strlen(l)&&l[pos]!='\"') pos++;
            t.type=STRING;
            t.value=NULL;
            t.value=malloc((pos-start)+1);
            size_t i, sp=0;
            for(i=start; i<pos; i++, sp++) {
                if(l[i]=='\\') {
                    i++;
                    switch(l[i]) {
                        case 'n': t.value[sp++]='\n'; break;
                        case 'b': t.value[sp++]='\b'; break;
                        case 'r': t.value[sp++]='\r'; break;
                        case 'a': t.value[sp++]='\a'; break;
                        case '\\': t.value[sp++]='\\'; break;
                        default: i--; break;
                    }
                    i++;
                }
                t.value[sp]=l[i];
            }
            t.value[sp]=0;
            pos++;
            cv_push(&a->file, &t);
            continue;
        }
        char c=l[pos++];
        switch(c) {
            case '=':
                t=(Token){ASSIGN, strdup("=")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '+':
                t=(Token){PLUS, strdup("+")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '-':
                t=(Token){MINUS, strdup("-")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '*':
                t=(Token){STAR, strdup("*")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '/':
                t=(Token){SLASH, strdup("/")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '{':
                t=(Token){LPAREN, strdup("{")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '}':
                t=(Token){RPAREN, strdup("}")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case ':':
                t=(Token){COLON, strdup(":")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '@':
                t=(Token){LCALL, strdup("@")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '(':
                t=(Token){LBRACE, strdup("(")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case ')':
                t=(Token){RBRACE, strdup(")")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case ',':
                t=(Token){COMMA, strdup(",")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '.':
                t=(Token){DOT, strdup(".")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '>':
                t=(Token){BT, strdup(">")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
            case '<':
                t=(Token){LT, strdup("<")};
                cv_push(&a->file, &t);
                t.value=NULL;
                break;
        }
    }
}
