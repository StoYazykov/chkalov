#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../chkalov/chkalov.h"

Slot println(size_t argc, Slot *argp) {
    puts("IN PRINTLN!");
    printf("%s\n", argp->value);
}
