#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#include <error.h>

#define CALLOC_BUF(name, size) returnErr(callocErr((void**)(&name), 1, size))

#define OPEN_FILE(var, name, mode)         \
    FILE* var = fopen(name, mode);         \
    if (var == NULL) return ERR_OPEN_FILE;

static int myMin(int a, int b) { return a < b ? a : b; }
static int myMax(int a, int b) { return a > b ? a : b; }

ErrEnum fileSize(FILE *file, long *siz);
ErrEnum callocErr(void **ptr, size_t count, size_t size);
ErrEnum readFile(const char* file_name, void** array, int* size);

int isZero(double num);
int strcmpToBracket(const char* lft, const char* rgt);

void printDouble(FILE* fout, double num);

#endif // UTILS_H