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
    asprintf(&s, "#(ps,(#(cl,BB)))");
    eval(s, out);
    asprintf(&s, "#(ps,##(cl,BB))");
    eval(s, out);
    asprintf(&s, "#(ps,#(cl,BB))");
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

    return 0;
}

