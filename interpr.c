#include "vm.h"

void initFrame(struct Frame *frame) {
    frame->locals = NULL;
    frame->localsSize = 0;

    frame->parent = NULL;

    frame->stackPtr = 0;
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
#define READA(am) &chunk.instr[ip += am]
#define READT(t) *(t *)READA(sizeof(t))
        Inst i = chunk.instr[ip++];
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
                (void) pop(frame);
            } break;

            case IT_JUMP: {
                ip = READT(uint32_t);
            } break;

            case IT_LGET: {
                const uint32_t id = READT(uint32_t);
                struct Val v = frame->locals[id];
                v.owned = false;
                push(frame, v);
            } break;

            case IT_LPUT: {
                const uint32_t id = READT(uint32_t);
                if (id >= frame->localsSize) {
                    frame->locals = realloc(frame->locals,
                                            sizeof(struct Val) * (id + 1));
                }
                struct Val v = pop(frame);
                moveOrCopy(&frame->locals[id], &v);
            }

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
            }
        }
    }
}
