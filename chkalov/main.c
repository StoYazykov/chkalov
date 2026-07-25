#include "../chkalov.h"
#include "parser.h"

int main(int argc, char **argv) {
    if(argc<3) {
        puts("Usage: chkc <input> <output> [flags]");
        puts("    flags: ");
        puts("  o - disable optimisation");
        puts("  d - out debug info");
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
    if(debug) printf("Done! \r\n");
    return 0;
}
