#include "vm.h"

struct Val arrayCreate(struct Val *data, size_t len) {
    struct Val *new = malloc(sizeof(struct Val) * len);
    for (size_t i = 0; i < len; i ++) {
        moveOrCopy(&new[i], &data[i]);
    }

    struct Val arr;
    arr.type = VT_ARR;
    arr.owned = true;
    arr.varr = (struct Array) {
        .elements = len,
        .arr = new
    };
    return arr;
}


struct Val arrayFromStrCopy(const char * const str, const size_t len) {
    struct Val *new = malloc(sizeof(struct Val) * len);
    for (size_t i = 0; i < len; i ++) {
        new[i] = charVal(str[i]);
    }

    struct Val arr;
    arr.type = VT_ARR;
    arr.owned = true;
    arr.varr = (struct Array) {
        .elements = len,
        .arr = new
    };
    return arr;
}