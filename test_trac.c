#include "trac.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main() {
    char *s;
    /* idle loop */
    asprintf(&s, "#(ps,#(rs))");
    eval(s);

    return 0;
}

