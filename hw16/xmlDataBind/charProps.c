#include "xml/details/genx/genx.h"
#include "xml/details/genx/charProps.h"

/* Simplified character property functions */
int isXMLChar(int c) {
    /* Basic ASCII check */
    return (c >= 0x20 && c <= 0x7E) || c == 0x9 || c == 0xA || c == 0xD;
}

int isInitialNameChar(int c) {
    /* Basic ASCII name start char check */
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

int isNameChar(int c) {
    /* Basic ASCII name char check */
    return isInitialNameChar(c) || (c >= '0' && c <= '9') || c == '-' || c == '.';
}

int isSpace(int c) {
    /* XML whitespace */
    return c == 0x20 || c == 0x9 || c == 0xA || c == 0xD;
}
