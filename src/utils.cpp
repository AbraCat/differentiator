#include <stdlib.h>
#include <math.h>

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

ErrEnum readFile(const char* file_name, void** array, int* size)
{
    myAssert(file_name != NULL && array != NULL && *array == NULL && size != NULL);

    FILE *fin = fopen(file_name, "r");
    if (fin == NULL) return ERR_OPEN_FILE;
    returnErr(fileSize(fin, (long*)size));
    ++*size;
    *array = calloc(1, *size);
    if (*array == NULL) return ERR_MEM;
    fread(*array, 1, *size - 1, fin);
    fclose(fin);
    (*((char**)array))[*size - 1] = 0;
    return ERR_OK;
}

int isZero(double num)
{
    const double EPS = 1e-1;
    return fabs(num) < EPS;
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