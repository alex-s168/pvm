#include "vm.h"

struct Val arrayCreate(struct Val *data, size_t len) {
    const struct Val arr = uninitializedArray(len);
    for (size_t i = 0; i < len; i ++) {
        moveOrCopy(&arr.varr.arr[i], &data[i]);
    }
    return arr;
}

struct Val arrayFromStrCopy(const char * const str, const size_t len) {
    const struct Val arr = uninitializedArray(len);
    for (size_t i = 0; i < len; i ++) {
        arr.varr.arr[i] = charVal(str[i]);
    }
    return arr;
}

void arrayJoin(struct Array *dest, const struct Array src) {
    dest->arr = realloc(dest->arr,
                        (dest->elements + src.elements) * sizeof(struct Val));
    memcpy(dest->arr + dest->elements,
           src.arr,
           src.elements * sizeof(struct Val));
    dest->elements += src.elements;
}

struct Val uninitializedArray(size_t size) {
    struct Array arr;
    arr.elements = size;
    arr.arr = malloc(sizeof(struct Val) * size);
    return arrayValOwned(arr);
}

struct Val arrayCopy(const struct Array src) {
    const struct Val dest = uninitializedArray(src.elements);
    for (size_t i = 0; i < src.elements; i ++) {
        refOrCopy(&dest.varr.arr[i], &src.arr[i]);
    }
    return dest;
}

struct Val arrayRevCopy(const struct Array src) {
    const struct Val dest = uninitializedArray(src.elements);
    for (size_t i = 0; i < src.elements; i ++) {
        refOrCopy(&dest.varr.arr[src.elements - i - 1], &src.arr[i]);
    }
    return dest;
}