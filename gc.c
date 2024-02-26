#include "vm.h"

struct ObjHeader *gcAlloc(const size_t size) {
    struct ObjHeader *alloc = malloc(size);
    if (alloc != NULL) {
        alloc->refs = 0;
        alloc->size = size;
    }
    return alloc;
}

void gcUse(struct ObjHeader *h) {
    h->refs ++;
}

void gcUnuse(struct ObjHeader *h) {
    h->refs --;
    if (h->refs == 0) {
        if (h->destroy_fun != NULL)
            h->destroy_fun(h);
        free(h);
    }
}