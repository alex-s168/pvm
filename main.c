#include "vm.h"
#include <stdio.h>

// #define CLASS_FILE "test_class_Test.h"
// #include "class.h"

int main() {
    // OBJ_T(Test) a = OBJ_ALLOC(Test);
    // OBJ_USE(a);
    // a->fname = arrayFromStrCopy("33", 2).varr;
    // OBJ_UNUSE(a);
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
    };
    struct Frame frame;
    initFrame(&frame);
    interpret(&frame, (struct InstChunk) { .instr = insts, .instrSize = sizeof(insts) });
    stackdump(&frame, stdout);
    destroyFrame(&frame);
    return 0;
}
