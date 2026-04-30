#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int64_t println(int64_t a) {
    printf("%s \n", (char *)a);
}

int64_t readln(int64_t a) {
    char *a=malloc(a);
    scanf("%s", a);
    return (int64_t)a;
}

int64_t memfree(int64_t a) {
    free((void *)a);
}
