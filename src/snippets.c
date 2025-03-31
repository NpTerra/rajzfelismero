#include "debugmalloc.h"
#include "snippets.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


char* strclone(const char *str)
{
    size_t n = strlen(str);
    char *s = malloc((n+1) * sizeof(char));
    strcpy(s, str);

    return s;
}


void memdump(char *filename, int line) {
    static int call = 0;
    DebugmallocData *instance = debugmalloc_singleton();
    debugmalloc_log("****************************************************\n"
                        "* File: %s:%d\n"
                        "* Hívás: %d Osszes foglalas: %d blokk, %d bajt.\n"
                        "****************************************************\n",
                        filename, line, call++,
                        instance->all_alloc_count, instance->all_alloc_bytes);
}


double distance(long long x1, long long y1, long long x2, long long y2)
{
    long long x = max(x1, x2) - min(x1, x2);
    long long y = max(y1, y2) - min(y1, y2);
    double r = sqrt(x*x + y*y);

    return r <= 2 ? ceil(r) : round(r);
}
