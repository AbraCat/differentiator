#include <stdlib.h>

#include <utils.h>

ErrEnum callocErr(void **ptr, size_t count, size_t size)
{
    void* ptr1 = calloc(count, size);
    if (ptr == NULL) return ERR_MEM;
    *ptr = ptr1;
    return ERR_OK;
}

ErrEnum fileSize(FILE *file, long *siz)
{
    myAssert(file != NULL && siz != NULL);

    if (fseek(file, 0L, SEEK_END))
    {
        *siz = -1L;
        return ERR_FILE;
    }

    *siz = ftell(file);
    if (*siz == -1L) return ERR_FILE;

    if (fseek(file, 0L, SEEK_SET)) return ERR_FILE;

    return ERR_OK;
}

int strcmpToBracket(const char* lft, const char* rgt)
{
    while (*lft != '(' && *lft != ')' && *lft != '\0' && *lft == *rgt)
    {
        ++lft;
        ++rgt;
    }
    return (*lft == '(' || *lft == ')' ? '\0' : *lft) - 
           (*rgt == '(' || *rgt == ')' ? '\0' : *rgt);
}