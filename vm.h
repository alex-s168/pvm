#ifndef VM_H
#define VM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DBG

#ifdef DBG
#include <assert.h>
#endif

enum ValType {
    VT_INT,
    VT_FLOAT,
    VT_CHAR,
    VT_BOOL,
    VT_OBJ,
    VT_ARR
};

struct ObjVarInfo {
    bool mutable;
    bool isStatic;
    enum ValType type;

    union {
        struct {
            size_t offset;
        } dyn;

        struct {
            struct Val *addr;
        } stat;
    };
};

/**
 * \brief Maps variable names to their offset and type in objects
 */
struct ObjMap {
    size_t                          len;
    const char ** const             names;
    const struct ObjVarInfo * const vars;
};

const struct ObjVarInfo *ObjMap_resolve(struct ObjMap *map, const char *var);

typedef void (*DestroyFun)(struct ObjHeader *);
typedef void (*CopyFun)(struct ObjHeader *dst, struct ObjHeader *src);

struct ObjHeader {
    size_t refs;
    /* total size of the object + this struct */
    size_t size;
    DestroyFun destroy_fun;
    CopyFun copy_fun;
    struct ObjMap *map;
};

struct Val *objVarByName(struct ObjHeader *obj, const char *var);
struct Val *objVarByInfo(struct ObjHeader *obj, const struct ObjVarInfo *info);

// TODO: null

// TODO: fnptr; callcontext;

struct Array {
    size_t elements;
    struct Val *arr;
};

static inline size_t ValType_size(const enum ValType type) {
    switch (type) {
        case VT_INT:
            return sizeof(long long int);
        case VT_FLOAT:
            return sizeof(double);
        case VT_BOOL:
        case VT_CHAR:
            return sizeof(char);
        case VT_OBJ:
            return sizeof(void *);
        case VT_ARR:
            return sizeof(struct Array);
        default:
            return UINT64_MAX;
    }
}

struct Val {
    bool owned;
    enum ValType type;
    union {
        long long int vint;
        double vfloat;
        char vchar;
        char vbool;
        struct ObjHeader *vobj;
        struct Array varr;
    };
};

struct ObjHeader *gcAlloc(const size_t size);
void gcUse(struct ObjHeader *h);
void gcUnuse(struct ObjHeader *h);

#define OBJ_ALLOC(claz) ((struct c##claz *) gcAlloc(sizeof(struct c##claz *)))
#define OBJ_T(claz) struct c##claz *
#define OBJ_USE(o) gcUse((struct ObjHeader *) (o))
#define OBJ_UNUSE(o) gcUse((struct ObjHeader *) (o))

void copy(struct Val *dest, const struct Val *src);
void moveOrCopy(struct Val *dest, const struct Val *src);
void refOrCopy(struct Val *dest, const struct Val *src);

void destroy(const struct Val v);

/**
 * If the value is an object and it implements [Displayable], it will call it's "toString()" function and return the returned array.
 * Otherwise:
 *      type  example
 *   float  ->        1.1
 *   int    ->          1
 *   array  ->  [1, 2, 3]
 *   bool   ->       true
 *   char   ->        'c'
 *   obj    -> Obj(0x249)
 */ 
struct Array tostring(struct Val v);

/**
 * Heap-allocates a C string and copies the data of the array over.
 * If any element in the array is not a char, it will return null.
 */
char *ascstring(struct Array a);

/**
 * Treats the array as a char array.
 * Prints every character to the stream.
 * If the element is not a char, cancel.
 */
void writeAsStr(struct Array a, FILE *stream);

/**
 * \brief Creates a new array on the heap and executes @see moveOrCopy for every element.
 * \param data
 * \param len
 * \return
 */
struct Val arrayCreate(struct Val *data, size_t len);

struct Val arrayFromStrCopy(const char * const str, const size_t len);

#define arrayFromCStr(str) arrayFromStrCopy(str, strlen(str))

static inline struct Val charVal(const char c) {
    return (struct Val) { .owned = true, .type = VT_CHAR, .vchar = c };
}

static inline struct Val boolVal(const bool b) {
    return (struct Val) { .owned = true, .type = VT_BOOL, .vbool = b };
}

static inline struct Val intVal(const long long int i) {
    return (struct Val) { .owned = true, .type = VT_INT, .vint = i };
}

static inline struct Val floatVal(const double f) {
    return (struct Val) { .owned = true, .type = VT_FLOAT, .vfloat = f };
}

static inline struct Val objValOwned(struct ObjHeader *obj) {
    return (struct Val) { .owned = true, .type = VT_OBJ, .vobj = obj };
}

static inline struct Val objValRef(struct ObjHeader *obj) {
    return (struct Val) { .owned = false, .type = VT_OBJ, .vobj = obj };
}


typedef uint8_t Inst;

struct InstChunk {
    uint32_t instrSize;
    Inst *instr;
};

struct Frame {
    struct Frame *parent;

    struct Val *stack;
    size_t stackPtr;

    struct Val *locals;
    uint32_t localsSize;
};

void initFrame(struct Frame *frame);
void destroyFrame(struct Frame *frame);

void interpret(struct Frame *frame, struct InstChunk chunk);

void stackdump(struct Frame *frame, FILE *stream);


#define IT_IMMF  (Inst) 0x00 // +8
#define IT_IMMI  (Inst) 0x01 // +8
#define IT_POP   (Inst) 0x02
#define IT_TRUE  (Inst) 0x03
#define IT_FALSE (Inst) 0x04
#define IT_IMMC  (Inst) 0x05 // +1
#define IT_ARR   (Inst) 0x06 // +4
#define IT_JUMP  (Inst) 0x07 // +4
#define IT_LGET  (Inst) 0x08 // +4
#define IT_LPUT  (Inst) 0x09 // +4
#define IT_ADD   (Inst) 0x0a


#define III_ARR_4(p) ((Inst *)p)[0], ((Inst *)p)[1], ((Inst *)p)[2], ((Inst *)p)[3]
#define III_ARR_8(p) III_ARR_4(p), III_ARR_4(((Inst *)p)+4)


#define I_IMMF(flt) IT_IMMF, III_ARR_8((double[]){(double)flt})
#define I_IMMI(i)   IT_IMMI, III_ARR_8((long long int[]){(long long int)i})
#define I_BOOL(b)   b ? IT_TRUE : IT_FALSE
#define I_POP()     IT_POP
#define I_IMMC(c)   IT_IMMC, (Inst) c
#define I_ARR(len)  IT_ARR, III_ARR_4((uint32_t[]){(uint32_t)len})
#define I_JUMP(tg)  IT_JUMP, III_ARR_4((uint32_t[]){(uint32_t)tg})
#define I_LGET(id)  IT_LGET, III_ARR_4((uint32_t[]){(uint32_t)id})
#define I_LPUT(id)  IT_LPUT, III_ARR_4((uint32_t[]){(uint32_t)id})
#define I_ADD()     IT_ADD


#endif //VM_H
