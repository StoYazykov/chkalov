#include "token.h"

void tok_init(Token *a, TokenType b, ds c) {
    a->value=NULL;
    ds_str(&a->value, c);
    a->type=b;
}
void tok_free(Token *a) {
    free(a->value);
}
