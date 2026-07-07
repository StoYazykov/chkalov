#include "../chkalov.h"

Slot println(size_t argc, Slot *argp) {
    if(ISSTR(argp->type)) puts(argp->value);
    else printf("%d\r\n", argp->value);
}
