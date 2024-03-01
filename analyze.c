#include "vm.h"

struct DynamicChunk analyze(struct InstChunk in,
                         size_t *inCount,
                         size_t *outCount,
                         size_t *varIntCount,
                         size_t *usedCount) {
    struct DynamicChunk dyn = dynChunkInit();

    int min = 0;
    int max = 0;
    int sp = 0;
    int nextArg = 0;

    uint32_t ip = 0;
    while (ip < in.instrSize) {

#define READA(am) &in.instr[(ip += am) - am]
#define READT(t) (*(t *)READA(sizeof(t)))
#define UPD(diff) sp += diff; \
if (sp < min) { \
for (size_t __ic = 0; __ic < abs(sp - min); __ic ++) { \
DynamicList_addAll(&dyn.instr, (Inst[]) { I_JITALYSDAT0(nextArg) }, 5); \
nextArg ++; \
} \
min = sp; \
}; \
if (sp > max) max = sp

        Inst i = READT(Inst);
        switch (i) {
            case IT_IMMF: {
                UPD(1);
                DynamicList_add(&dyn.instr, &i);
                DynamicList_addAll(&dyn.instr, &READT(double), sizeof(double));
            } break;

            case IT_IMMI: {
                UPD(1);
                DynamicList_add(&dyn.instr, &i);
                DynamicList_addAll(&dyn.instr, &READT(long long int), sizeof(long long int));
            } break;

            case IT_IMMC: {
                UPD(1);
                DynamicList_add(&dyn.instr, &i);
                DynamicList_addAll(&dyn.instr, &READT(char), sizeof(char));
            } break;

            case IT_NULL:
            case IT_TRUE:
            case IT_FALSE: {
                UPD(1);
                DynamicList_add(&dyn.instr, &i);
            } break;

            case IT_POP: {
                UPD(-1);
                DynamicList_add(&dyn.instr, &i);
            } break;

            case IT_LPUT: {
                UPD(-1);
                DynamicList_add(&dyn.instr, &i);
                DynamicList_addAll(&dyn.instr, &READT(uint32_t), sizeof(uint32_t));
            } break;

            case IT_LGET: {
                UPD(1);
                DynamicList_add(&dyn.instr, &i);
                DynamicList_addAll(&dyn.instr, &READT(uint32_t), sizeof(uint32_t));
            } break;

            case IT_LCLEAR: {
                DynamicList_add(&dyn.instr, &i);
                DynamicList_addAll(&dyn.instr, &READT(uint32_t), sizeof(uint32_t));
            } break;

           case IT_EQNUM:
            case IT_GTNUM:
            case IT_LTNUM:
            case IT_SUB:
            case IT_ADD: {
                UPD(-2);
                UPD(1);
                DynamicList_add(&dyn.instr, &i);
            } break;

            case IT_ARR: {
                uint32_t size = READT(uint32_t);
                UPD(-size);
                DynamicList_add(&dyn.instr, &i);
                DynamicList_addAll(&dyn.instr, &size, sizeof(uint32_t));
            } break;

            case IT_NZNUM:
            case IT_REV:
            case IT_REVR: {
                UPD(-1);
                UPD(1);
                DynamicList_add(&dyn.instr, &i);
            } break;

            case IT_REF:
            case IT_COPY: {
                UPD(-1);
                UPD(2);
                DynamicList_add(&dyn.instr, &i);
            } break;

            case IT_JUMP:
            case IT_BRANCH:
            default:
                ip --;
                goto lbreak;
        }

        continue;

        lbreak:
            break;
    }

    *usedCount = ip;
    *inCount = -min;
    *outCount = sp - min;
    *varIntCount = max - min - 1;

    return dyn;
}
