#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../chkalov/chkalov.h"

Slot println(size_t argc, Slot *argp) {
    //puts("IN PRINTLN!");
    //printf("argp->type: %x\r\n", argp->type);
    if(ISSTR(argp->type)) printf("%s\n", argp->value);
    else printf("%d\n", argp->value);
}
