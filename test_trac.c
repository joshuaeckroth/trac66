#include "trac.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

int main() {
    char *s;
    const char *rval;

    /* asserts, don't work, need to set up output buffer */

    asprintf(&s, "#(ds,AA,CAT)");
    rval = eval(s);
    assert(rval == NULL);
    free(s);
    asprintf(&s, "#(ds,BB,(#(cl,AA)))");
    rval = eval(s);
    assert(rval == NULL);
    free(s);
    asprintf(&s, "#(ps,(#(cl,BB)))");
    rval = eval(s);
    assert(strcmp(rval, "#(cl,BB)") == 0);
    free(s);
    asprintf(&s, "#(ps,##(cl,BB))");
    rval = eval(s);
    assert(strcmp(rval, "#(cl,AA)") == 0);
    free(s);
    asprintf(&s, "#(ps,#(cl,BB))");
    rval = eval(s);
    assert(strcmp(rval, "CAT") == 0);
    free(s);
    
    /* idle loop */
    /*
    asprintf(&s, "#(ps,#(rs))");
    eval(s);
    free(s);
    */

    return 0;
}

