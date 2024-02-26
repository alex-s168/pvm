#define T_ARR   struct Array
#define T_INT   long long int
#define T_FLOAT double
#define T_BOOL  char
#define T_CHAR  char
#define T_OBJ   struct ObjHeader *

#define CLASS(name) struct c##name
#define HEADER struct ObjHeader header;
#define FIELD(type, name) type f##name;
#define MUT_FIELD(type, name) FIELD(type, name)
#define OFFSET(o) /* */
#define STATIC_FIELD(type, name) /* */
#define ADDR(a) /* */
#ifdef CLASS_FILE
#include CLASS_FILE
#else
CLASS_CODE
#endif
#undef CLASS
#undef HEADER
#undef FIELD
#undef MUT_FIELD
#undef OFFSET
#undef STATIC_FIELD
#undef ADDR

#define CLASS(name) static const char ** const cmn##name =
#define HEADER /* */
#define FIELD(type, name) #name,
#define MUT_FIELD(type, name) FIELD(type, name)
#define OFFSET(o) /* */
#define STATIC_FIELD(type, name) /* */
#define ADDR(a) /* */
#ifdef CLASS_FILE
#include CLASS_FILE
#else
CLASS_CODE
#endif
#undef FIELD
#undef CLASS
#undef HEADER
#undef MUT_FIELD
#undef OFFSET
#undef STATIC_FIELD
#undef ADDR

#undef T_ARR
#undef T_INT
#undef T_FLOAT
#undef T_BOOL
#undef T_CHAR
#undef T_OBJ

#define T_ARR   VT_ARR
#define T_INT   VT_INT
#define T_FLOAT VT_FLOAT
#define T_BOOL  VT_BOOL
#define T_CHAR  VT_CHAR
#define T_OBJ   VT_OBJ

#define CLASS(name) static const struct ObjVarInfo * const cmt##name = (struct ObjVarInfo[])
#define HEADER /* */
#define FIELD(typein, name) (struct ObjVarInfo) { .mutable = false, .isStatic = false, .type = typein, .dyn = { .offset =
#define MUT_FIELD(typein, name) (struct ObjVarInfo) { .mutable = true, .isStatic = false, .type = typein, .dyn = { .offset =
#define STATIC_FIELD(typein, name) (struct ObjVarInfo) { .mutable = true, .isStatic = true, .type = typein, .stat = { .addr =
#define ADDR(addr) addr } }
#define OFFSET(o) o } },
#ifdef CLASS_FILE
#include CLASS_FILE
#else
CLASS_CODE
#endif
#undef FIELD
#undef CLASS
#undef HEADER
#undef MUT_FIELD
#undef STATIC_FIELD
#undef OFFSET
#undef ADDR

#undef T_ARR
#undef T_INT
#undef T_FLOAT
#undef T_BOOL
#undef T_CHAR
#undef T_OBJ

#define CLASS(name) struct ObjMap cm##name = (struct ObjMap) { \
.len = sizeof(cmt##name) / sizeof(struct ObjVarInfo), \
.names = cmn##name, \
.vars = cmt##name, \
}; struct cmee##name
#define HEADER /* */
#define FIELD(type, name) /* */,
#define MUT_FIELD(type, name) /* */
#define OFFSET(o) /* */
#define STATIC_FIELD(type, name) /* */
#define ADDR(a) /* */
#ifdef CLASS_FILE
#include CLASS_FILE
#else
CLASS_CODE
#endif
#undef FIELD
#undef CLASS
#undef HEADER
#undef MUT_FIELD
#undef OFFSET
#undef STATIC_FIELD
#undef ADDR