#include "vm.h"

void disasm(struct InstChunk chunk, FILE *stream) {
    uint32_t ip = 0;
    while (ip < chunk.instrSize) {

#define READA(am) &chunk.instr[(ip += am) - am]
#define READT(t) (*(t *)READA(sizeof(t)))
#define SIMPLE(id, str) case id: { fputs(str, stream); } break;

        fprintf(stream, " %u ", ip);

        const Inst i = READT(Inst);
        switch (i) {
            case IT_IMMF: {
                double val = READT(double);
                fprintf(stream , " immf %f\n", val);
            } break;

            case IT_IMMI: {
                long long int val = READT(long long int);
                fprintf(stream , " immi %llu\n", val);
            } break;

            case IT_IMMC: {
                char val = READT(char);
                fprintf(stream , " immc %c\n", val);
            } break;

            SIMPLE(IT_TRUE, " true\n");

            SIMPLE(IT_FALSE, " false\n");

            SIMPLE(IT_NULL, " null\n");

            SIMPLE(IT_POP, " pop\n");

            case IT_JUMP: {
                uint32_t tg = READT(uint32_t);
                fprintf(stream , " jump %u\n", tg);
            } break;

            case IT_LGET: {
                const uint32_t id = READT(uint32_t);
                fprintf(stream , " lget %u\n", id);
            } break;

            case IT_LPUT: {
                const uint32_t id = READT(uint32_t);
                fprintf(stream , " lput %u\n", id);
            } break;

            SIMPLE(IT_ADD, " add\n")

            SIMPLE(IT_SUB, " sub\n")

            case IT_ARR: {
                uint32_t len = READT(uint32_t);
                fprintf(stream , " arr %u\n", len);
            } break;

            SIMPLE(IT_REV, " rev\n")

            SIMPLE(IT_REVR, " revr\n")

            SIMPLE(IT_REF, " ref\n")

            case IT_BRANCH: {
                uint32_t tg = READT(uint32_t);
                fprintf(stream , " branch %u\n", tg);
            } break;

            SIMPLE(IT_EQNUM, " eqnum\n")

            SIMPLE(IT_GTNUM, " gtnum\n")

            SIMPLE(IT_LTNUM, " ltnum\n")

            SIMPLE(IT_NZNUM, " nznum\n")

            case IT_JITALYSDAT0: {
                uint32_t arg = READT(uint32_t);
                fprintf(stream , " analysisdata_arg %u\n", arg);
            } break;

            default:
                fputs(" unknown\n", stream);
                break;
        }
    }
}