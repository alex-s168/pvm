#include "vm.h"
#include "gen.h"

#include <stdio.h>
#include <time.h>

// #define CLASS_FILE "test_class_Test.h"
// #include "class.h"

int main() {
    // OBJ_T(Test) a = OBJ_ALLOC(Test);
    // OBJ_USE(a);
    // a->fname = arrayFromStrCopy("33", 2).varr;
    // OBJ_UNUSE(a);

    /*
    Inst insts[] = {
        I_IMMF(1.1),
        I_LPUT(0),
        I_ADD(),
        I_ADD(),
        I_LPUT(2),
        I_LGET(2),
    };
    struct InstChunk chunk;
    initInstChunk(&chunk, sizeof(insts), insts);

    size_t inCount;
    size_t outCount;
    size_t varIntCount;
    size_t usedCount;
    struct DynamicChunk analyzed = analyze(chunk, &inCount, &outCount, &varIntCount, &usedCount);

    printf(" inputs: %zu\n outputs: %zu\n used internal stack positions: %zu\n processed: %zu / %u\n",
        inCount, outCount, varIntCount, usedCount, chunk.instrSize);

    printf("\ndiasm:\n");

    disasm(dynChunkAs(analyzed), stdout);

    DynamicList_clear(&analyzed.instr);
*/

    
    struct FunGenCtx main;
    startFunGen(&main);

    struct GenBlock start;
    struct GenBlock loop;
    const int i = 0;
    const int a = 1;
    const int b = 2;
    const int c = 3;
    const int d = 4;

    {
        startBlockGen(&start);

        blockGenInstrsEz(&start, (Inst[]) { I_IMMI(10000000) });
        blockGenInstrsEz(&start, (Inst[]) { I_LPUT(i) });

        //blockGenInstrsEz(&start, (Inst[]) { IT_JUMP });
        //    blockGenRef(&start, &loop);

        endBlockGen(&start, &main);
    }

    {
        startBlockGen(&loop);

        blockGenInstrsEz(&loop, (Inst[]) { I_IMMI(1) });
        blockGenInstrsEz(&loop, (Inst[]) { I_LGET(i) });
        blockGenInstrsEz(&loop, (Inst[]) { I_SUB() });
        blockGenInstrsEz(&loop, (Inst[]) { I_LPUT(i) });

        Inst instr[] = {
            I_IMMI(9),
            I_IMMI(5),
            I_ADD(),
            I_LPUT(a),

            I_LGET(a),
            I_IMMI(1),
            I_SUB(),
            I_LPUT(b),

            I_LGET(a),
            I_LGET(b),
            I_ADD(),
            I_LPUT(c),

            I_LGET(c),
            I_LGET(b),
            I_SUB(),
            I_LPUT(d),

            I_LGET(d),
            I_POP(),
        };

        blockGenInstrs(&loop, instr, sizeof(instr));

        blockGenInstrsEz(&loop, (Inst[]) { I_LGET(i) });
        blockGenInstrsEz(&loop, (Inst[]) { I_NZNUM() });
        blockGenInstrsEz(&loop, (Inst[]) { IT_BRANCH });
            blockGenRef(&loop, &loop);

        endBlockGen(&loop, &main);
    }

    struct InstChunk chunk = endFunGen(&main);
    destroyFunGen(&main);

    fputs("Instruction chunk:\n ", stdout);
    dumpChunk(chunk, stdout);
    disasm(chunk, stdout);
    fputc('\n', stdout);

    struct Frame frame;
    struct LocalFrame locals;
    initFrame(&frame, true);
    initLocalFrame(&locals);

    clock_t t = clock();
    interpret(&frame, &locals, &chunk);
    t = clock() - t;
    double seconds = (double) t / CLOCKS_PER_SEC;
    printf("Execution time: %fs\n\n", seconds);


    localdump(&locals, stdout);
    framedump(&frame, stdout);
    destroyFrame(&frame);
    destroyLocalFrame(&locals);
    fputc('\n', stdout);
    gcStats(stdout);

    free(chunk.instr);

    return 0;
}
