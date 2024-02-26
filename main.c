#include "vm.h"
#include <stdio.h>

#define CLASS_FILE "test_class_Test.h"
#include "class.h"

int main() {
    OBJ_T(Test) a = OBJ_ALLOC(Test);
    OBJ_USE(a);
    a->fname = arrayFromStrCopy("33", 2).varr;
    OBJ_UNUSE(a);
    Inst insts[] = {
        I_IMMF(3.0),
        I_LPUT(0),
        I_IMMF(1.0),
        I_LGET(0),
        I_ADD(),
    };
    struct Frame frame;
    initFrame(&frame);
    // interpret(&frame, insts);
    return 0;
}
