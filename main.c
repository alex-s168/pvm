#include "vm.h"
#include <stdio.h>

// #define CLASS_FILE "test_class_Test.h"
// #include "class.h"

int main() {
    // OBJ_T(Test) a = OBJ_ALLOC(Test);
    // OBJ_USE(a);
    // a->fname = arrayFromStrCopy("33", 2).varr;
    // OBJ_UNUSE(a);
    fputs("z\n", stdout);
    Inst insts[] = {
        I_IMMF(3.0),
        I_LPUT(0),
        I_IMMF(1.0),
        I_LGET(0),
        I_ADD(),
    };
    for (size_t i = 0; i < sizeof(insts); i ++) {
        printf("%uu ", insts[i]);
    }
    fputs("\n\na\n", stdout);
    struct Frame frame;
    initFrame(&frame);
    fputs("b\n", stdout);
    interpret(&frame, (struct InstChunk) { .instr = insts, .instrSize = sizeof(insts) });
    fputs("c\n", stdout);
    stackdump(&frame, stdout);
    fputs("d\n", stdout);
    destroyFrame(&frame);
    return 0;
}
