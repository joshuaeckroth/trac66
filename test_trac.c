#include "trac.h"
/* _GNU_SOURCE for asprintf */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main() {
    FILE *out = fopen("test.out", "w");

    char *s;

    asprintf(&s, "#(ds,AA,CAT)");
    eval(s, out);
    asprintf(&s, "#(ds,BB,(#(cl,AA)))");
    eval(s, out);
    asprintf(&s, "#(ps,foo)");
    eval(s, out);
    asprintf(&s, "#(ps,(#(cl,BB)))");
    eval(s, out);
    asprintf(&s, "#(ps,##(cl,BB))");
    eval(s, out);
    asprintf(&s, "#(ps,#(cl,BB))");
    eval(s, out);
    asprintf(&s, "#(ss,AA,A,C))#(ps,#(cl,AA,Q,R))");
    eval(s, out);
    asprintf(&s, "#(ds,CC,CATCATCAT)");
    eval(s, out);
    asprintf(&s, "#(ss,CC,A,C))#(ps,#(cl,CC,Q,R))");
    eval(s, out);
    asprintf(&s, "#(ps,#(eq,A,A,B,C))");
    eval(s, out);
    asprintf(&s, "#(ps,#(eq,A,B,B,C))");
    eval(s, out);
    asprintf(&s, "#(ps,#(ad,1.5,2.0,Z))");
    eval(s, out);
    asprintf(&s, "#(ps,#(ml,1.5,2.0,Z))");
    eval(s, out);
    asprintf(&s, "#(ds,Factorial,(#(eq,1,X,1,(#(ml,X,#(cl,Factorial,#(ad,X,-1)))))))#(ss,Factorial,X)#(ps,#(cl,Factorial,5))");
    eval(s, out);

    fclose(out);

    pid_t child = fork();
    if(child == 0) {
        char *argv[] = {"/usr/bin/diff", "-y", "test.gold", "test.out", 0};
        int result = execv("/usr/bin/diff", argv);
        if(result == -1) {
            perror("Error:");
        }
    } else {
        int wstatus;
        pid_t result = wait(&wstatus);
        if(result == child) {
            if(WEXITSTATUS(wstatus) == 0) {
                printf("\nAll tests passed.\n");
            } else {
                printf("\nFailures: %d\n", WEXITSTATUS(wstatus));
            }
        } else {
            perror("Error:");
        }
    }
    free_toc();

    return 0;
}

