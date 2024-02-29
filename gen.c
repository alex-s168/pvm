#include "gen.h"

void startFunGen(struct FunGenCtx *ctx) {
    ctx->blocks = NULL;
    ctx->blocksCount = 0;
}

void startBlockGen(struct GenBlock *block) {
    block->instr = NULL;
    block->size = 0;
    block->locationResolved = false;
    block->refs = NULL;
    block->refsCount = 0;
}

void endBlockGen(struct GenBlock *block, struct FunGenCtx *parent) {
    parent->blocks = realloc(parent->blocks,
                             sizeof(struct GenBlock *) * (parent->blocksCount + 1));
    parent->blocks[parent->blocksCount ++] = block;
}

struct InstChunk endFunGen(struct FunGenCtx *ctx) {
    size_t total = 0;
    for (size_t i = 0; i < ctx->blocksCount; i ++) {
        total += ctx->blocks[i]->size;
    }
    Inst *out = malloc(total * sizeof(Inst));

    uint32_t pt = 0;
    for (size_t i = 0; i < ctx->blocksCount; i ++) {
        struct GenBlock *block = ctx->blocks[i];
        block->locationResolved = true;
        block->location = pt;
        memcpy(out + pt, block->instr, block->size);
        pt += block->size;
    }

    for (size_t i = 0; i < ctx->blocksCount; i ++) {
        const struct GenBlock *block = ctx->blocks[i];
        for (size_t r = 0; r < block->refsCount; r ++) {
            const struct GenRef ref = block->refs[r];
            Inst *where = out + block->location + ref.location;
            *(uint32_t *)where = ref.target->location;
        }
    }

    return (struct InstChunk) { .instr = out, .instrSize = total };
}

void destroyFunGen(struct FunGenCtx *ctx) {
    for (size_t i = 0; i < ctx->blocksCount; i ++) {
        const struct GenBlock *block = ctx->blocks[i];
        free(block->instr);
        free(block->refs);
    }
    free(ctx->blocks);
}

void blockGenInstrs(struct GenBlock *block, const Inst *inst, size_t size) {
    block->instr = realloc(block->instr, block->size + size);
    memcpy(block->instr + block->size, inst, size);
    block->size += size;
}

void blockGenRef(struct GenBlock *block, struct GenBlock *refDest) {
    block->refs = realloc(block->refs,
                          (block->refsCount + 1) * sizeof(struct GenRef));
    block->refs[block->refsCount ++] = (struct GenRef) { .target = refDest, .location = block->size };
    blockGenInstrs(block, (const Inst[]) { 0, 0, 0, 0 }, 4);
}