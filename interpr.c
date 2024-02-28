#include "vm.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

void initFrame(struct Frame *frame) {
    frame->locals = NULL;
    frame->localsSize = 0;

    frame->parent = NULL;

    frame->stack = NULL;
    frame->stackPtr = 0;
}

void stackdump(struct Frame *frame, FILE *stream) {
    fputs("<<<\n", stream);
    for (size_t i = 0; i < frame->stackPtr; i ++) {
        fputc(' ', stream);
        const struct Val elem = frame->stack[i];
        const struct Array tostr = tostring(elem);
        writeAsStr(tostr, stream);
        destroyArr(tostr);
        fputc('\n', stream);
    }
    fputs(">>>\n", stream);
}

void destroyFrame(struct Frame *frame) {
    for (size_t i = 0; i < frame->stackPtr; i ++) {
        destroy(frame->stack[i]);
    }
    free(frame->stack);

    for (size_t i = 0; i < frame->localsSize; i ++) {
        destroy(frame->locals[i]);
    }
    free(frame->locals);
}

static void push(struct Frame *frame, struct Val val) {
    frame->stack = realloc(frame->stack,
                           sizeof(struct Val) * (frame->stackPtr + 1));
    frame->stack[frame->stackPtr ++] = val;
}

static struct Val pop(struct Frame *frame) {
    const struct Val val = frame->stack[-- frame->stackPtr];
    frame->stack = realloc(frame->stack,
                           sizeof(struct Val) * frame->stackPtr);
    return val;
}

inline static struct Val peek(const struct Frame *frame) {
    return frame->stack[frame->stackPtr - 1];
}

inline static bool has(const struct Frame *frame, const size_t elems) {
    return frame->stackPtr >= elems;
}

void interpret(struct Frame *frame, struct InstChunk chunk) {
    uint32_t ip = 0;
    while (ip < chunk.instrSize) {
#define READA(am) &chunk.instr[(ip += am) - am]
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
                struct Val v = frame->locals[id];
                v.owned = false;
                push(frame, v);
            } break;

            case IT_LPUT: {
                const uint32_t id = READT(uint32_t);
                if (!has(frame, 1))
                    break;
                if (id >= frame->localsSize) {
                    frame->locals = realloc(frame->locals,
                                            sizeof(struct Val) * (id + 1));
                }
                frame->localsSize = id + 1;
                struct Val v = pop(frame);
                moveOrCopy(&frame->locals[id], &v);
            } break;

            case IT_ADD: {
                if (!has(frame, 2))
                    break;

                const struct Val a = pop(frame);
                const struct Val b = pop(frame);

                struct Val res;
                if (a.type == VT_INT && b.type == VT_INT) {
                    res = intVal(a.vint + b.vint);
                } else if (a.type == VT_INT && b.type == VT_FLOAT) {
                    res = floatVal((double) a.vint + b.vfloat);
                } else if (a.type == VT_FLOAT && b.type == VT_INT) {
                    res = floatVal(a.vfloat + (double) b.vint);
                } else if (a.type == VT_FLOAT && b.type == VT_FLOAT) {
                    res = floatVal(a.vfloat + b.vfloat);
                } else {
                    res = nullVal();
                }

                push(frame, res);

                cleanup:
                    destroy(a);
                    destroy(b);
            } break;

            case IT_ARR: {
                const uint32_t len = READT(uint32_t);
                if (!has(frame, len))
                    break;
                struct Val *start = frame->stack + frame->stackPtr - len;
                frame->stackPtr -= len;
                const struct Val arr = arrayCreate(start, len);
                frame->stack = realloc(frame->stack,
                                       (frame->stackPtr + 1) * sizeof(struct Val));
                frame->stack[frame->stackPtr] = arr;
                frame->stackPtr ++;
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
                const struct Val other = peek(frame);
                struct Val new;
                refOrCopy(&new, &other);
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

            default: {
                assert(false);
            } break;
        }
    }
}
