#include "token.h"
#include <iostream>

Token::Token(TokenType __type, string __value) {
    type = __type;
    value = __value;
}
