#include <stdio.h>
#include <stdlib.h>

#include <error.h>
#include <colors.h>

// generate enum automatically
ErrDescr err_arr[] = {{ERR_OK, "No error"}, 

{ERR_MEM, "Couldn't allocate memory"},
{ERR_OPEN_FILE, "Couldn't open file"},
{ERR_FILE, "File error"}};

const int n_errs = (sizeof err_arr) / sizeof(ErrDescr);

void myAssertFn(int expr, const char* str_expr, const char* file, int line, const char* function)
{
    if (expr)
        return;

    printf("%sAssertion failed: %s at %s:%d (%s)%s\n", RED_STR, str_expr, file, line, function, DEFAULT_STR);
    exit(expr);
}

void getErrDescr(ErrEnum num, const char** descr)
{
    for (int i = 0; i < n_errs; ++i)
    {
        if (err_arr[i].num == num)
        {
            *descr = err_arr[i].descr;
            return;
        }
    }
    *descr = NULL;
}

void handleErrFn(ErrEnum num, const char* file, int line, const char* function)
{
    if (num == ERR_OK)
        return;

    const char* descr = NULL;
    getErrDescr(num, &descr);

    if (descr == NULL)
        printf("%sUnknown error (%d) at %s:%d (%s)%s\n", RED_STR, (int)num, file, line, function, DEFAULT_STR);
    else
        printf("%sError: %s at %s:%d (%s)%s\n", RED_STR, descr, file, line, function, DEFAULT_STR);

    exit(num);
}