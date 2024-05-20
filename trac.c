#include "trac.h"
/* _GNU_SOURCE for asprintf */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *const argv[])
{
    int opt = 0;
    int use_stdlib = 1;
    while((opt = getopt(argc, argv, "nh")) != -1) {
        switch(opt) {
            case 'n':
                use_stdlib = 0;
                break;
            case 'h':
            default:
                fprintf(stderr, "Usage: %s [-n]\n\n\t-n\tDon't use stdlib\n\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    printf("TRAC66. Terminate inputs with ' (the \"meta\" character).\n\n");

    tochead = NULL;

    if(use_stdlib) {
        printf("Using standard library.\n\n");
        // stdlib.trac limit to 32k text at this moment
        FILE *stdlib_in = fopen("stdlib.trac", "r");
        if(stdlib_in) {
            char *stdlib_text = (char*)malloc(1024*32);
            if(!stdlib_text) {
                fprintf(stderr, "Cannot allocate space for reading stdlib.trac!\n");
                exit(EXIT_FAILURE);
            }
            size_t read_bytes = fread(stdlib_text, 1, 1024*32-1, stdlib_in);
            printf("Read %d bytes\n", read_bytes);
            stdlib_text[read_bytes] = '\0';
            printf("Evaluating stdlib.trac... ");
            eval(stdlib_text, stdout);
            printf("done.\n");
            debug_print_toc();
            printf("\n\n");
        }
    }

    char *s = NULL;
    do {
        asprintf(&s, "#(ps,#(rs))");
    } while(eval(s, stdout) != (const char*)-1);

    return 0;
}

