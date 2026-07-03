#include "../chkalov.h"
#include "parser.h"

int main(int argc, char **argv) {
    if(argc < 3) {
        printf("Usage: chkc input.chk output.cvm\n");
        return 1;
    }
    if(!getenv("CHKALOV")) {
        printf("CHKALOV not set!\n");
        return 1;
    }

    Parser parser;
printf("Before par_init...\n");
par_init(&parser, argv[1], argv[2], false);
printf("Before par_parFile...\n");
par_parFile(&parser);
printf("Before par_free...\n");
par_free(&parser);
printf("Done!\n");
    return 0;
}
