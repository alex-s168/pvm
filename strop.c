#include "vm.h"

#include <stdio.h>

struct Array tostring(struct Val v) {
    // TODO: Displayable interface
    static char buf[64];
    switch (v.type) {
        case VT_INT: {
            sprintf(buf, "%lli", v.vint);
            return arrayFromCStr(buf).varr;
        } break;

        case VT_FLOAT: {
            sprintf(buf, "%f", v.vfloat);
            return arrayFromCStr(buf).varr;
        } break;

        case VT_CHAR: {
            sprintf(buf, "\"%c\"", v.vchar);
            return arrayFromCStr(buf).varr;
        } break;

        case VT_BOOL: {
            return arrayFromCStr(v.vbool ? "true" : "false").varr;
        } break;

        case VT_OBJ: {
            sprintf(buf, "%s(%p)", v.vobj->map->name, v.vobj);
            return arrayFromCStr(buf).varr;
        }

        case VT_ARR: {
            struct Array out = arrayFromCStr("[").varr;
            const struct Array seperator = arrayFromCStr(", ").varr;
            for (size_t i = 0; i < v.varr.elements; i ++) {
                if (i != 0) {
                    arrayJoin(&out, seperator);
                }
                const struct Array el = tostring(v.varr.arr[i]);
                arrayJoin(&out, el);
                destroyArr(el);
            }
            const struct Array end = arrayFromCStr("]").varr;
            arrayJoin(&out, end);
            destroyArr(seperator);
            destroyArr(end);
            return out;
        }

        default:
            return arrayFromCStr("Unknown").varr;
    }
}

char *ascstring(struct Array a) {
    char *str = malloc(a.elements + 1);
    if (str == NULL)
        goto err_ret;

    for (size_t i = 0; i < a.elements; i ++) {
        struct Val v = a.arr[i];
        if (v.type != VT_CHAR)
            goto error;
        str[i] = v.vchar;
        if (v.vchar == '\0')
            break;
    }
    str[a.elements] = '\0';

    return str;

error:
    free(str);
err_ret:
    return NULL;
}

void writeAsStr(struct Array a, FILE *stream) {
    for (size_t i = 0; i < a.elements; i ++) {
        struct Val v = a.arr[i];
        if (v.type != VT_CHAR)
            break;
        fputc(v.vchar, stream);
    }
}
