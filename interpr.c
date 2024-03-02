#include "vm.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void initFrame(struct Frame *frame, bool extraDbg) {
    frame->extraDbg = extraDbg;

    Ally ally;
    if (extraDbg)
        ally = getStatAlloc(getLIBCAlloc(), &frame->stackAllocStats);
    else
        ally = getLIBCAlloc();

    DynamicList_init(&frame->stack,
                     sizeof(struct Val),
                     ally,
                     4);
}

void initLocalFrame(struct LocalFrame *frame) {
    frame->data = NULL;
    frame->size = 0;
}

void destroyLocalFrame(struct LocalFrame *frame) {
    for (size_t i = 0; i < frame->size; i ++) {
        destroy(frame->data[i]);
    }
    free(frame->data);
}

void localdump(struct LocalFrame *locals, FILE *stream) {
    fputs("Locals:\n", stream);
    for (size_t i = 0; i < locals->size; i ++) {
        fprintf(stream, " %zu: ", i);
        const struct Val elem = locals->data[i];
        const struct Array tostr = tostring(elem);
        writeAsStr(tostr, stream);
        destroyArr(tostr);
        fputc('\n', stream);
    }
}

void framedump(struct Frame *frame, FILE *stream) {
    if (frame->extraDbg) {
        fputs("Stack allocation statistics:\n", stream);
        outputStats(&frame->stackAllocStats, stream);
        fputc('\n', stream);
    }

    fputs("\nStack:\n<<<\n", stream);
    for (size_t i = 0; i < frame->stack.fixed.len; i ++) {
        fputc(' ', stream);
        const struct Val *elem = FixedList_get(frame->stack.fixed, i);
        const struct Array tostr = tostring(*elem);
        writeAsStr(tostr, stream);
        destroyArr(tostr);
        fputc('\n', stream);
    }
    fputs(">>>\n", stream);
}

void destroyFrame(struct Frame *frame) {
    for (size_t i = 0; i < frame->stack.fixed.len; i ++) {
        struct Val *elem = FixedList_get(frame->stack.fixed, i);
        destroy(*elem);
    }
    DynamicList_clear(&frame->stack);
}

static void push(struct Frame *frame, struct Val val) {
    DynamicList_add(&frame->stack, &val);
}

static struct Val pop(struct Frame *frame) {
    // TODO: replace with future removeLast
    const size_t i = frame->stack.fixed.len - 1;
    const struct Val val = *(struct Val *)FixedList_get(frame->stack.fixed, i);
    DynamicList_removeAt(&frame->stack, i);
    return val;
}

inline static struct Val peek(const struct Frame *frame) {
    const size_t i = frame->stack.fixed.len - 1;
    const struct Val *val = FixedList_get(frame->stack.fixed, i);
    return *val;
}

inline static bool has(const struct Frame *frame, const size_t elems) {
    return frame->stack.fixed.len >= elems;
}

#define BINARY_OP_START(id) case id: { \
if (!has(frame, 2)) \
break; \
const struct Val a = pop(frame); \
const struct Val b = pop(frame); \
struct Val res;

#define BINARY_OP_II (a.type == VT_INT && b.type == VT_INT)
#define BINARY_OP_IF (a.type == VT_INT && b.type == VT_FLOAT)
#define BINARY_OP_FF (a.type == VT_FLOAT && b.type == VT_FLOAT)
#define BINARY_OP_FI (a.type == VT_FLOAT && b.type == VT_INT)

#define BINARY_OP_END push(frame, res); \
destroy(a); \
destroy(b); \
} break;


#define UNARY_OP_START(id) case id: { \
if (!has(frame, 1)) \
break; \
const struct Val a = pop(frame); \
struct Val res;

#define UNARY_OP_F (a.type == VT_FLOAT)
#define UNARY_OP_I (a.type == VT_INT)

#define UNARY_OP_END push(frame, res); \
destroy(a); \
} break;

// #define STUPID_LOCALS

void interpret(struct Frame *frame, struct LocalFrame *locals, struct InstChunk *chunk) {
    if (!chunk->basicAnalyzed)
        analyzeBasic(chunk);

#ifndef STUPID_LOCALS
    uint32_t oldLocalsSize = locals->size;
    uint32_t localsSize = chunk->localsCount;
    if (localsSize > oldLocalsSize) {
        locals->size = localsSize;
        locals->data = realloc(locals->data, localsSize * sizeof(struct Val));
        for (size_t i = oldLocalsSize; i < localsSize; i ++) {
            locals->data[i] = nullVal();
        }
    }
#endif

    uint32_t ip = 0;
    while (ip < chunk->instrSize) {
#define READA(am) &chunk->instr[(ip += am) - am]
#define READT(t) (*(t *)READA(sizeof(t)))
        Inst i = READT(Inst);
        switch (i) {
            case IT_IMMF: {
                push(frame, floatVal(READT(double)));
            } break;

            case IT_IMMI: {
                push(frame, intVal(READT(long long int)));
            } break;

            case IT_IMMC: {
                push(frame, charVal(READT(char)));
            } break;

            case IT_TRUE: {
                push(frame, boolVal(true));
            } break;

            case IT_FALSE: {
                push(frame, boolVal(false));
            } break;

            case IT_NULL: {
                push(frame, nullVal());
            } break;

            case IT_POP: {
                if (has(frame, 1))
                    destroy(pop(frame));
            } break;

            case IT_JUMP: {
                uint32_t tg = READT(uint32_t);
                ip = tg;
            } break;

            case IT_LGET: {
                const uint32_t id = READT(uint32_t);
#ifdef STUPID_LOCALS
                if (id >= locals->size) {
                    push(frame, nullVal());
                    break;
                }
#endif
                struct Val v = locals->data[id];
                v.owned = false;
                push(frame, v);
            } break;

            case IT_LPUT: {
                const uint32_t id = READT(uint32_t);
                if (!has(frame, 1))
                    break;
#ifdef STUPID_LOCALS
                if (id >= locals->size) {
                    locals->data = realloc(locals->data,
                                           sizeof(struct Val) * (id + 1));
                    locals->size = id + 1;
                }
#endif
                struct Val v = pop(frame);
                destroy(locals->data[i]);
                moveOrCopy(&locals->data[id], &v);
            } break;

            case IT_LCLEAR: {
                const uint32_t id = READT(uint32_t);
#ifdef STUPID_LOCALS
                if (id >= locals->size)
                    break;
#endif
                destroy(locals->data[id]);
                locals->data[id] = nullVal();
            } break;

            BINARY_OP_START(IT_ADD)
                if BINARY_OP_II {
                    res = intVal(a.vint + b.vint);
                } else if BINARY_OP_IF {
                    res = floatVal((double) a.vint + b.vfloat);
                } else if BINARY_OP_FI {
                    res = floatVal(a.vfloat + (double) b.vint);
                } else if BINARY_OP_FF {
                    res = floatVal(a.vfloat + b.vfloat);
                } else {
                    res = nullVal();
                }
            BINARY_OP_END

            BINARY_OP_START(IT_SUB)
                if BINARY_OP_II {
                    res = intVal(a.vint - b.vint);
                } else if BINARY_OP_IF {
                    res = floatVal((double) a.vint - b.vfloat);
                } else if BINARY_OP_FI {
                    res = floatVal(a.vfloat - (double) b.vint);
                } else if BINARY_OP_FF {
                    res = floatVal(a.vfloat - b.vfloat);
                } else {
                    res = nullVal();
                }
            BINARY_OP_END

            case IT_ARR: {
                const uint32_t len = READT(uint32_t);
                if (!has(frame, len))
                    break;
                struct Val arr;
                if (len > 0) {
                    const size_t index = frame->stack.fixed.len - len;
                    struct Val *start = FixedList_get(frame->stack.fixed, index);
                    arr = arrayCreate(start, len);
                    DynamicList_removeRange(&frame->stack, index, frame->stack.fixed.len - 1);
                } else {
                    arr = arrayEmpty();
                }
                push(frame, arr);
            } break;

            case IT_REV: {
                if (!has(frame, 1))
                    goto it_rev_bad;
                const struct Val old = pop(frame);
                if (old.type != VT_ARR)
                    goto it_rev_bad;
                const struct Val new = arrayRevCopy(old.varr);
                push(frame, new);
                destroy(old);
                break;

                it_rev_bad:
                    destroy(old);
                    push(frame, nullVal());
            } break;

            case IT_REVR: {
                if (!has(frame, 1))
                    goto it_revr_bad;
                const struct Val other = peek(frame);
                if (other.type != VT_ARR)
                    goto it_revr_bad;
                const struct Val new = arrayRevCopy(other.varr);
                push(frame, new);
                break;

                it_revr_bad:
                    push(frame, nullVal());
            } break;

            case IT_COPY: {
                if (!has(frame, 1)) {
                    push(frame, nullVal());
                    break;
                }
                const struct Val other = peek(frame);
                struct Val new;
                copy(&new, &other);
                push(frame, new);
            } break;

            case IT_REF: {
                if (!has(frame, 1)) {
                    push(frame, nullVal());
                    break;
                }
                struct Val other = peek(frame);
                struct Val new;
                ref(&new, &other);
                push(frame, new);
            } break;

            case IT_BRANCH: {
                uint32_t tg = READT(uint32_t);
                if (!has(frame, 1))
                    break;
                const struct Val v = pop(frame);
                if (v.type == VT_BOOL)
                    if (v.vbool)
                        ip = tg;
                destroy(v);
            } break;

            BINARY_OP_START(IT_EQNUM)
                if BINARY_OP_II {
                    res = boolVal(a.vint == b.vint);
                } else if BINARY_OP_IF {
                    res = boolVal((double) a.vint == b.vfloat);
                } else if BINARY_OP_FI {
                    res = boolVal(a.vfloat == (double) b.vint);
                } else if BINARY_OP_FF {
                    res = boolVal(a.vfloat == b.vfloat);
                } else {
                    res = nullVal();
                }
            BINARY_OP_END

            BINARY_OP_START(IT_GTNUM)
                if BINARY_OP_II {
                    res = boolVal(a.vint > b.vint);
                } else if BINARY_OP_IF {
                    res = boolVal((double) a.vint > b.vfloat);
                } else if BINARY_OP_FI {
                    res = boolVal(a.vfloat > (double) b.vint);
                } else if BINARY_OP_FF {
                    res = boolVal(a.vfloat > b.vfloat);
                } else {
                    res = nullVal();
                }
            BINARY_OP_END

            BINARY_OP_START(IT_LTNUM)
                if BINARY_OP_II {
                    res = boolVal(a.vint < b.vint);
                } else if BINARY_OP_IF {
                    res = boolVal((double) a.vint < b.vfloat);
                } else if BINARY_OP_FI {
                    res = boolVal(a.vfloat < (double) b.vint);
                } else if BINARY_OP_FF {
                    res = boolVal(a.vfloat < b.vfloat);
                } else {
                    res = nullVal();
                }
            BINARY_OP_END

            UNARY_OP_START(IT_NZNUM)
                if UNARY_OP_F {
                    res = boolVal(a.vfloat != 0);
                } else if UNARY_OP_I {
                    res = boolVal(a.vint != 0);
                } else {
                    res = nullVal();
                }
            UNARY_OP_END

            case IT_JITALYSDAT0: {
                (void) READT(uint32_t);
            } break;

            case IT_HINT_BLOCK_BEGIN:
            case IT_HINT_BLOCK_END:
            case IT_HINT_OPTIMIZE:
                break;

            default: {
                assert(false);
            } break;
        }
    }
}
