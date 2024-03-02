#include "vm.h"
#include <assert.h>
#include <stdint.h>

void analyzeBasic(struct InstChunk *inst) {
	int64_t max = -1;

    uint32_t ip = 0;
    while (ip < inst->instrSize) {

#define READA(am) &inst->instr[(ip += am) - am]
#define READT(t) (*(t *)READA(sizeof(t)))

        Inst i = READT(Inst);
        switch (i) {
            case IT_IMMF: {
           		(void) READT(double);
            } break;

            case IT_IMMI: {
                (void) READT(long long int);
             } break;

            case IT_IMMC: {
                (void) READT(char);
            } break;

            case IT_NULL:
            case IT_TRUE:
            case IT_FALSE:
            case IT_POP:
            case IT_HINT_BLOCK_BEGIN:
            case IT_HINT_BLOCK_END:
            case IT_HINT_OPTIMIZE:
            case IT_EQNUM:
            case IT_GTNUM:
            case IT_LTNUM:
            case IT_SUB:
            case IT_ADD:
 			case IT_NZNUM:
            case IT_REV:
            case IT_REVR:
            case IT_REF:
            case IT_COPY:
           		break;

            case IT_LPUT:
            case IT_LGET:
            case IT_LCLEAR: {
            	uint32_t id = READT(uint32_t);
            	if (id > max)
            		max = id;
            } break;

            case IT_ARR: {
                (void) READT(uint32_t);
            } break;

            case IT_JUMP:
            case IT_BRANCH: {
            	(void) READT(uint32_t);
            } break;

            default:
            	assert(false);
        }
    }

    inst->basicAnalyzed = true;
    inst->localsCount = max + 1;
}