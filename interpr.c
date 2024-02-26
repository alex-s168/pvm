#include "vm.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>

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
        struct Val elem = frame->stack[i];
        struct Array tostr = tostring(elem);
        writeAsStr(tostr, stream);
        free(tostr.arr);
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
    struct Val val = frame->stack[frame->stackPtr --];
    frame->stack = realloc(frame->stack,
                           sizeof(struct Val) * frame->stackPtr);
    return val;
}

inline static struct Val peek(const struct Frame *frame) {
    return frame->stack[frame->stackPtr - 1];
}

void interpret(struct Frame *frame, struct InstChunk chunk) {
    uint32_t ip = 0;
    while (ip < chunk.instrSize) {
        uint32_t lastIp = ip;
#define READA(am) &chunk.instr[ip += am]
#define READT(t) *(t *)READA(sizeof(t))
        Inst i = READT(Inst);
        printf("at ip %ul: %uu\n  ", lastIp, i);
        for (size_t ii = ip; ii < chunk.instrSize; ii ++) {
            printf("%ul ", chunk.instr[ii]);
        }
        printf("\n");
        // raise(SIGTRAP);
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

            case IT_POP: {
                destroy(pop(frame));
            } break;

            case IT_JUMP: {
                uint32_t tg = READT(uint32_t);
                ip = tg;
            } break;

            case IT_LGET: {
                const uint32_t id = READT(uint32_t);
                printf("get id %ul\n", id);
                struct Val v = frame->locals[id];
                v.owned = false;
                push(frame, v);
            } break;

            case IT_LPUT: {
                const uint32_t id = READT(uint32_t);
                printf("put id %ul\n", id);
                if (id >= frame->localsSize) {
                    frame->locals = realloc(frame->locals,
                                            sizeof(struct Val) * (id + 1));
                }
                struct Val v = pop(frame);
                moveOrCopy(&frame->locals[id], &v);
            } break;

            case IT_ADD: {
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
                    goto cleanup;
                }

                push(frame, res);

                cleanup:
                    destroy(a);
                    destroy(b);
            } break;

            case IT_ARR: {
                assert(false);
            } break;

            default: {
                assert(false);
            } break;
        }
    }
}
