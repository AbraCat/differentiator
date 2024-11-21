#include <stdlib.h>

#include <logs.h>

FILE* fdump = NULL;
extern const char log_path[] = "./log";
static const char fdump_path[] = "";
static const int buffer_size = 300;

ErrEnum openDumpFile()
{
    if (fdump == NULL)
    {
        char buf[buffer_size] = "";
        sprintf(buf, "%s/dump/dump.html", log_path);

        fdump = fopen(buf, "w");
        if (fdump == NULL) return ERR_OPEN_FILE;

        if (atexit(closeDumpFile) != 0) return ERR_ATEXIT;
    }
    return ERR_OK;
}

void closeDumpFile()
{
    if (fdump != NULL) fclose(fdump);
}