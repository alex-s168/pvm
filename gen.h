#ifndef GEN_H
#define GEN_H

#include "vm.h"

struct FunGenCtx {
    struct GenBlock **blocks;
    size_t            blocksCount;
};

struct GenRef {
    struct GenBlock *target;
    size_t           location;
};

struct GenBlock {
    Inst  *instr;
    size_t size;

    bool     locationResolved;
    uint32_t location;

    struct GenRef *refs;
    size_t refsCount;
};

void startFunGen(struct FunGenCtx *ctx);

void startBlockGen(struct GenBlock *block);

void endBlockGen(struct GenBlock *block, struct FunGenCtx *parent);

struct InstChunk endFunGen(struct FunGenCtx *ctx);

void destroyFunGen(struct FunGenCtx *ctx);

void blockGenInstrs(struct GenBlock *block, const Inst *inst, size_t size);

#define blockGenInstrsEz(block, instrs) blockGenInstrs(block, instrs, sizeof(instrs))

void blockGenRef(struct GenBlock *block, struct GenBlock *refDest);

#endif //GEN_H
