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
        I_IMMF(3.0),
        I_LPUT(0),
        I_IMMF(1.0),
        I_LGET(0),
        I_ADD(),

        I_IMMF(1),
        I_IMMF(2),
        I_IMMF(3),
        I_ARR(3),

        I_ARR(2),
    };
    struct InstChunk chunk = { .instr = insts, .instrSize = sizeof(insts) };
    */

    struct FunGenCtx main;
    startFunGen(&main);

    struct GenBlock start;
    struct GenBlock loop;
    const int i = 0;

    {
        startBlockGen(&start);

        blockGenInstrsEz(&start, (Inst[]) { I_IMMI(100000) });
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

        blockGenInstrsEz(&loop, (Inst[]) { I_LGET(i) });
        blockGenInstrsEz(&loop, (Inst[]) { I_NZNUM() });
        blockGenInstrsEz(&loop, (Inst[]) { IT_BRANCH });
            blockGenRef(&loop, &loop);

        endBlockGen(&loop, &main);
    }

    struct InstChunk chunk = endFunGen(&main);
    destroyFunGen(&main);

    dumpChunk(chunk, stdout);

    struct Frame frame;
    initFrame(&frame);

    clock_t t = clock();
    interpret(&frame, chunk);
    t = clock() - t;
    double seconds = (double) t / CLOCKS_PER_SEC;
    printf("Execution time: %fs\n", seconds);


    stackdump(&frame, stdout);
    destroyFrame(&frame);
    gcStats(stdout);

    free(chunk.instr);

    return 0;
}
