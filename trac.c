#include "trac.h"
/* _GNU_SOURCE for asprintf */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("TRAC66. Terminate inputs with ' (the \"meta\" character).\n\n");
    tochead = NULL;

    char *s = NULL;
    do {
        asprintf(&s, "#(ps,#(rs))");
    } while(eval(s) != (const char*)-1);

    return 0;
}

