#ifndef ERROR_H
#define ERROR_H

// #define MY_NDEBUG

#define handleErr(err_num) handleErrFn(err_num, __FILE__, __LINE__, __FUNCTION__)
    
#define returnErr(expr)     \
{                           \
    ErrEnum err_num = expr; \
    if (err_num)            \
        return err_num;     \
}

#ifdef MY_NDEBUG
#define myAssert(expr)
#else
#define myAssert(expr) myAssertFn(expr, #expr, __FILE__, __LINE__, __FUNCTION__)
#endif


enum ErrEnum
{
    ERR_OK = 0,
    ERR_UNKNOWN,

    ERR_MEM,
    ERR_OPEN_FILE,
    ERR_FILE,
    ERR_BUF_BOUND,
    ERR_ATEXIT,

    ERR_NULL_TREE,
    ERR_INVAL_CONNECT,
    ERR_TREE_FMT,
    ERR_NOT_LEAF,
    ERR_PARENT_DISCARDED,
    ERR_TREE_CYCLE,

    ERR_INVAL_OP_CODE,
    ERR_INVAL_OP_STR,
};

struct ErrDescr
{
    ErrEnum num;
    const char* descr;
};

void myAssertFn(int expr, const char* str_expr, const char* file, int line, const char* function);
void getErrDescr(ErrEnum num, const char** descr);
void handleErrFn(ErrEnum num, const char* file, int line, const char* function);

#endif // ERROR_H