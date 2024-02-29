#include "vm.h"
#include "gen.h"

#include <stdio.h>

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

    struct GenBlock b1;
    struct GenBlock b2;

    {
        startBlockGen(&b1);

        blockGenInstrsEz(&b1, (Inst[]) { I_IMMF(2.2) });

        blockGenInstrsEz(&b1, (Inst[]) { IT_JUMP });
            blockGenRef(&b1, &b2);

        endBlockGen(&b1, &main);
    }

    {
        startBlockGen(&b2);

        blockGenInstrsEz(&b2, (Inst[]) { I_IMMF(1.1) });

        endBlockGen(&b2, &main);
    }

    struct InstChunk chunk = endFunGen(&main);
    destroyFunGen(&main);

    dumpChunk(chunk, stdout);

    struct Frame frame;
    initFrame(&frame);
    interpret(&frame, chunk);
    stackdump(&frame, stdout);
    destroyFrame(&frame);
    gcStats(stdout);

    free(chunk.instr);

    return 0;
}
