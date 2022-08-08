#include "trac.h"
/* _GNU_SOURCE for asprintf */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    freopen("test.out", "w", stdout);

    char *s;

    asprintf(&s, "#(ds,AA,CAT)");
    eval(s);
    /* assert(rval == NULL); */
    asprintf(&s, "#(ds,BB,(#(cl,AA)))");
    eval(s);
    /* assert(rval == NULL); */
    asprintf(&s, "#(ps,(#(cl,BB)))");
    eval(s);
    /* assert(strcmp(rval, "#(cl,BB)") == 0); */
    asprintf(&s, "#(ps,##(cl,BB))");
    eval(s);
    /* assert(strcmp(rval, "#(cl,AA)") == 0); */
    asprintf(&s, "#(ps,#(cl,BB))");
    eval(s);
    /* assert(strcmp(rval, "CAT") == 0); */

    pid_t child = fork();
    if(child == 0) {
        char *argv[] = {"/usr/bin/diff", "test.gold", "test.out", 0};
        execv("/usr/bin/diff", argv);
    } else {
        int wstatus;
        wait(&wstatus);
        fprintf(stderr, "Done.\n");
    }

    return 0;
}

