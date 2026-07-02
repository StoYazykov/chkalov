#include <stdio.h>
#include "../chkalov.h"

int main(void) {
    error("A! %d", 52);
    cv v;
    cv_init(&v, 16, sizeof(int));
    int a=5;
    cv_push(&v, &a);
    a=2;
    cv_push(&v, &a);
    a=0;
    cv_pop(&v, &a);
    printf("Number: %d\r\n", a);
    cv_pop(&v, &a);
    printf("Number: %d\r\n", a);
    cv_free(&v);
    return 0;
}
