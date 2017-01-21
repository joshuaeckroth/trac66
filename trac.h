#ifndef TRAC_H
#define TRAC_H

#define READ_STRING_DELIM '\''
#define MAX_STRING_SIZE 1024

#define DELETED 127 /* ASCII DEL (delete) */
#define ARGPTR 30 /* ASCII RS (record separator) */
#define ACTIVEFUNC 2 /* ASCII STX (start of text) */
#define NEUTRALFUNC 1 /* ASCII SOH (start of heading) */
#define FUNCEND 3 /* ASCII ETX (end of text) */

/* table of contents */

#define MAX_TOC_NAME_SIZE 256

typedef struct tocentry {
    char *name;
    char *val;
    struct tocentry *next;
} tocentry;

tocentry *tochead;
void define_string(const char *name, const char *val);
const char *get_string(const char *name);
void free_toc();
void debug_print_toc();

/* eval funcs */

const char *eval_define_string(const char *name, const char *val);
const char *eval_call_string(const char *name);
const char *eval_read_string();
const char *eval_print_string(const char *s);
const char *func_dispatch(char *ns, int start, int end);
const char *eval(char *s);

#endif


