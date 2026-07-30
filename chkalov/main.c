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

#include "../chkalov.h"
#include "parser.h"

int main(int argc, char **argv) {
    puts("Chkalov Compiler (version " VERSION ")\r\n");
    if(argc<3) {
        puts("Usage: chkc <input> <output> [flags]");
        puts("    flags: ");
        puts("  o - disable optimisation");
        puts("  d - out debug info\r\n");
        return 1;
    }
    if(!getenv("CHKALOV")) {
        printf("CHKALOV not set! \r\n");
        return 1;
    }
    //char *a=argv[3];
    int debug, optim;
    debug=0;
    optim=1;
    if(argc>3) {
        debug=!!strchr(argv[3], 'd');
        optim=!(!!strchr(argv[3], 'o'));
    }
    Parser parser;
    if(debug) printf("Before par_init... \r\n");
    par_init(&parser, argv[1], argv[2], debug);
    if(debug) printf("Before par_parFile... \r\n");
    par_parFile(&parser);
    if(optim) {
        if(debug) {
            puts("\r\nAST Tree before optimization:");
            ast_print((AstNode *)parser.root, 0);
        }
        par_optim(&parser);
        if(debug) {
            puts("\r\nAST Tree after optimization:");
            ast_print((AstNode *)parser.root, 0);
        }
    } else if(debug) {
        puts("\r\nAST Tree:");
        ast_print((AstNode *)parser.root, 0);
    }
    codegen(parser.root, &parser);
    if(debug) printf("\r\nBefore par_free... \r\n");
    par_free(&parser);
    printf("Compilation succesfull! \r\n");
    return 0;
}
