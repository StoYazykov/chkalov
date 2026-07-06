#include "../chkalov.h"

Slot println(size_t argc, Slot *argp) {
    puts("IN PRINTLN!");
    printf("%d\n", argp->value);
}
