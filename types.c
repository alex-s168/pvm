#include <string.h>

#include "vm.h"

void copy(struct Val *dest, const struct Val *src) {
    dest->type = src->type;
    switch (src->type) {
        case VT_INT:
            dest->vint = src->vint;
        break;

        case VT_FLOAT:
            dest->vfloat = src->vfloat;
        break;

        case VT_BOOL:
        case VT_CHAR:
            dest->vchar = src->vchar;
        break;

        case VT_OBJ: {
            struct ObjHeader *old = src->vobj;
            struct ObjHeader *new = gcAlloc(old->size);
            new->destroy_fun = old->destroy_fun;
            new->copy_fun = old->copy_fun;
            old->copy_fun(new, old);
            gcUse(new);
            dest->vobj = new;
        } break;

        case VT_ARR: {
            const size_t elems = src->varr.elements;
            dest->varr.elements = elems;
            dest->varr.arr = malloc( sizeof(struct Val) * elems);
            for (size_t i = 0; i < elems; i ++) {
                copy(&dest->varr.arr[i], &src->varr.arr[i]);
            }
        } break;
#ifdef DBG
        default:
            assert(false);
#endif
    }
}

void moveOrCopy(struct Val *dest, const struct Val *src) {
#ifdef DBG
    if (src->type == VT_OBJ)
        assert(src->owned);
#endif
    if (src->owned) {
        *dest = *src;
    }
    else {
        copy(dest, src);
        destroy(*src);
    }
}

void refOrCopy(struct Val *dest, const struct Val *src) {
    if (src->type == VT_OBJ) {
        gcUse(src->vobj);
        *dest = *src;
    } else {
        copy(dest, src);
    }
}

void destroyArr(const struct Array a) {
    for (size_t i = 0; i < a.elements; i ++) {
        destroy(a.arr[i]);
    }
    free(a.arr);
}

void destroy(const struct Val v) {
    if (!v.owned)
        return;

    if (v.type == VT_OBJ) {
        gcUnuse(v.vobj);
    }
    else if (v.type == VT_ARR) {
        destroyArr(v.varr);
    }
}

const struct ObjVarInfo *ObjMap_resolve(struct Class *map, const char *var) {
    for (size_t i = 0; i < map->len; i ++) {
        if (strcmp(map->names[i], var) == 0) {
            return &map->vars[i];
        }
    }
    return NULL;
}

struct Val *objVarByName(struct ObjHeader *obj, const char *var) {
    const struct ObjVarInfo *v = ObjMap_resolve(obj->map, var);
    if (v == NULL)
        return NULL;
    return objVarByInfo(obj, v);
}

struct Val *objVarByInfo(struct ObjHeader *obj, const struct ObjVarInfo *info) {
    if (info->isStatic)
        return info->stat.addr;
    return (struct Val *) ((char *) obj + info->dyn.offset);
}

bool classChildOf(struct Class *clazz, const char *parent) {
    for (size_t i = 0; i < clazz->superCount; i ++)
        if (strcmp(clazz->super[i], parent) == 0)
            return true;

    return false;
}

bool objInstanceOf(struct ObjHeader *obj, const char *clazz) {
    if (strcmp(obj->map->name, clazz) == 0)
        return true;

    return classChildOf(obj->map, clazz);
}