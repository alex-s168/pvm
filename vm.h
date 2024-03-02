#ifndef VM_H
#define VM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define _INC_STDIO
#include "kollektions/kallok.h"
#include "kollektions/lists.h"

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
    VT_ARR,
    VT_NULL,
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
struct Class {
          size_t              const len;
    const char **             const names;
    const struct ObjVarInfo * const vars;
    const char *              const name;

    const char **             const super;
          size_t              const superCount;
};

const struct ObjVarInfo *ObjMap_resolve(struct Class *map, const char *var);

typedef void (*DestroyFun)(struct ObjHeader *);
typedef void (*CopyFun)(struct ObjHeader *dst, struct ObjHeader *src);

struct ObjHeader {
    size_t refs;
    /* total size of the object + this struct */
    size_t size;
    DestroyFun destroy_fun;
    CopyFun copy_fun;
    struct Class *map;
};

struct Val *objVarByName(struct ObjHeader *obj, const char *var);
struct Val *objVarByInfo(struct ObjHeader *obj, const struct ObjVarInfo *info);

bool classChildOf(struct Class *clazz, const char *parent);
bool objInstanceOf(struct ObjHeader *obj, const char *clazz);

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
        case VT_NULL:
            return 0;
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
void gcStats(FILE *stream);

#define OBJ_ALLOC(claz) ((struct c##claz *) gcAlloc(sizeof(struct c##claz *)))
#define OBJ_T(claz) struct c##claz *
#define OBJ_USE(o) gcUse((struct ObjHeader *) (o))
#define OBJ_UNUSE(o) gcUse((struct ObjHeader *) (o))

void copy(struct Val *dest, const struct Val *src);

/**
 * \brief Moves the given value into the given destination.
 * If the source value is not owned, it creates a copy of it and destroys the source value.
 * Sets the source value to null afterwards.
 */
void moveOrCopy(struct Val *dest, const struct Val *src);

void refOrCopy(struct Val *dest, const struct Val *src);

void ref(struct Val *dest, struct Val *src);

void destroyArr(const struct Array a);
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

#define arrayEmpty() arrayCreate(NULL, 0)

struct Val arrayFromStrCopy(const char * const str, const size_t len);

struct Val uninitializedArray(size_t size);

struct Val arrayCopy(const struct Array src);

void arrayJoin(struct Array *dest, const struct Array src);

struct Val arrayRevCopy(const struct Array src);

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

static inline struct Val objVal(struct ObjHeader *obj) {
    return (struct Val) { .owned = true, .type = VT_OBJ, .vobj = obj };
}

static inline struct Val nullVal() {
    return (struct Val) { .owned = true, .type = VT_NULL };
}

static inline struct Val arrayValOwned(struct Array arr) {
    return (struct Val) { .owned = true, .type = VT_ARR, .varr = arr };
}

static inline struct Val arrayValRef(struct Array arr) {
    return (struct Val) { .owned = false, .type = VT_ARR, .varr = arr };
}


typedef uint8_t Inst;

struct InstChunk {
    uint32_t instrSize;
    Inst *instr;

    bool basicAnalyzed;
    uint32_t localsCount;
};

static void initInstChunk(struct InstChunk *chunk, uint32_t size, Inst *inst) {
    chunk->instrSize = size;
    chunk->instr = inst;
    chunk->basicAnalyzed = false;
}

static void dumpChunk(struct InstChunk chunk, FILE *stream) {
    for (size_t i = 0; i < chunk.instrSize; i ++) {
        fprintf(stream, "%02hhX ", chunk.instr[i]);
    }
    fprintf(stream, "\n");
}

struct DynamicChunk {
    struct DynamicList instr;
};

static struct DynamicChunk dynChunkInit() {
    struct DynamicChunk c;
    DynamicList_init(&c.instr,
                     sizeof(Inst),
                     KALLOK_PREFIX getLIBCAlloc(),
                     0);
    return c;
}

static struct InstChunk dynChunkAs(struct DynamicChunk dyn) {
    return (struct InstChunk) {
        .instrSize = (uint32_t) dyn.instr.fixed.len,
        .instr = (Inst *) dyn.instr.fixed.data, 
        .basicAnalyzed = false,
    };
}

struct LocalFrame {
    struct Val *data;
    uint32_t size;
};

struct Frame {
    bool extraDbg;

    struct KALLOK_PREFIX AllyStats stackAllocStats;
    /* struct Val * */
    struct DynamicList stack;
};

void initLocalFrame(struct LocalFrame *frame);
void destroyLocalFrame(struct LocalFrame *frame);

void initFrame(struct Frame *frame, bool extraDbg);
void destroyFrame(struct Frame *frame);

void analyzeBasic(struct InstChunk *inst);

void interpret(struct Frame *frame, struct LocalFrame *locals, struct InstChunk *chunk);

void framedump(struct Frame *frame, FILE *stream);
void localdump(struct LocalFrame *locals, FILE *stream);

struct DynamicChunk analyze(struct InstChunk in,
                         size_t *inCount,
                         size_t *outCount,
                         size_t *varIntCount,
                         size_t *usedCount);

void disasm(struct InstChunk chunk, FILE *stream);


#define IT_IMMF   (Inst) 0x00 // +8
#define IT_IMMI   (Inst) 0x01 // +8
#define IT_POP    (Inst) 0x02
#define IT_TRUE   (Inst) 0x03
#define IT_FALSE  (Inst) 0x04
#define IT_IMMC   (Inst) 0x05 // +1
#define IT_ARR    (Inst) 0x06 // +4
#define IT_JUMP   (Inst) 0x07 // +4
#define IT_LGET   (Inst) 0x08 // +4
#define IT_LPUT   (Inst) 0x09 // +4
#define IT_ADD    (Inst) 0x0a
#define IT_REV    (Inst) 0x0b
#define IT_NULL   (Inst) 0x0c
#define IT_COPY   (Inst) 0x0d
#define IT_REVR   (Inst) 0x0e
#define IT_REF    (Inst) 0x0f
#define IT_BRANCH (Inst) 0x10 // +4
#define IT_EQNUM  (Inst) 0x11
#define IT_GTNUM  (Inst) 0x12
#define IT_LTNUM  (Inst) 0x13
#define IT_NZNUM  (Inst) 0x14
#define IT_SUB    (Inst) 0x15
#define IT_JITALYSDAT0      (Inst) 0x16 // +4 
#define IT_LCLEAR           (Inst) 0x17 // +4
#define IT_HINT_BLOCK_BEGIN (Inst) 0x18
#define IT_HINT_BLOCK_END   (Inst) 0x19
#define IT_HINT_OPTIMIZE    (Inst) 0x1a


#define III_ARR_4(p) ((Inst *)p)[0], ((Inst *)p)[1], ((Inst *)p)[2], ((Inst *)p)[3]
#define III_ARR_8(p) III_ARR_4(p), III_ARR_4(((Inst *)p)+4)


#define I_IMMF(flt)  IT_IMMF, III_ARR_8((double[]){(double)flt})
#define I_IMMI(i)    IT_IMMI, III_ARR_8((long long int[]){(long long int)i})
#define I_BOOL(b)    b ? IT_TRUE : IT_FALSE
#define I_POP()      IT_POP
#define I_IMMC(c)    IT_IMMC, (Inst) c
#define I_ARR(len)   IT_ARR, III_ARR_4((uint32_t[]){(uint32_t)len})
#define I_JUMP(tg)   IT_JUMP, III_ARR_4((uint32_t[]){(uint32_t)tg})
#define I_LGET(id)   IT_LGET, III_ARR_4((uint32_t[]){(uint32_t)id})
#define I_LPUT(id)   IT_LPUT, III_ARR_4((uint32_t[]){(uint32_t)id})
#define I_ADD()      IT_ADD
#define I_REV()      IT_REV
#define I_NULL()     IT_NULL
#define I_REVR()     IT_REVR
#define I_COPY()     IT_COPY
#define I_REF()      IT_REF
#define I_BRANCH(tg) IT_BRANCH, III_ARR_4((uint32_t[]){(uint32_t)tg})
#define I_EQNUM()    IT_EQNUM
#define I_GTNUM()    IT_GTNUM
#define I_LTNUM()    IT_LTNUM
#define I_NZNUM()    IT_NZNUM
#define I_SUB()      IT_SUB
#define I_JITALYSDAT0(data)     IT_JITALYSDAT0, III_ARR_4((uint32_t[]){(uint32_t)data})
#define I_LCLEAR(id)            IT_LCLEAR, III_ARR_4((uint32_t[]){(uint32_t)id})
#define I_HINT_BLOCK_BEGIN()    IT_HINT_BLOCK_BEGIN
#define I_HINT_BLOCK_END()      IT_HINT_BLOCK_END
#define I_HINT_OPTIMIZE()       IT_HINT_OPTIMIZE


#define INSTRS(...)  ((Inst[]) { __VA_ARGS__ })


#endif //VM_H
