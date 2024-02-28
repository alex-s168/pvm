#include "vm.h"

static size_t statAllocs = 0;
static size_t statDeallocs = 0;

static size_t statRefIncs = 0;
static size_t statRefDecs = 0;

struct ObjHeader *gcAlloc(const size_t size) {
    struct ObjHeader *alloc = malloc(size);
    if (alloc != NULL) {
        alloc->refs = 0;
        alloc->size = size;
    }
    statAllocs ++;
    return alloc;
}

void gcUse(struct ObjHeader *h) {
    h->refs ++;
    statRefIncs ++;
}

void gcUnuse(struct ObjHeader *h) {
    h->refs --;
    statRefDecs --;
    if (h->refs == 0) {
        if (h->destroy_fun != NULL)
            h->destroy_fun(h);
        free(h);
        statDeallocs ++;
    }
}

void gcStats(FILE *stream) {
    const bool ok = statAllocs == statDeallocs &&
                    statRefIncs == statRefDecs;
    fprintf(stream, "GC: %s\n", ok ? "OK" : "BAD");
    fprintf(stream, " allocations: %zu\n deallocations: %zu\n", statAllocs, statDeallocs);
    fprintf(stream, " use: %zu\n unuse: %zu\n", statRefIncs, statRefDecs);
}