#include "trac.h"
/* _GNU_SOURCE for asprintf */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

tocentry *tochead;

void print_ns(const char *ns) {
    int i = 0;
    while(ns[i] != 0) {
        if(ns[i] == DELETED) {
            printf("_");
        } else if(ns[i] == ARGPTR) {
            printf("^");
        } else if(ns[i] == ACTIVEFUNC) {
            printf("!");
        } else if(ns[i] == NEUTRALFUNC) {
            printf("~");
        } else if(ns[i] == FUNCEND) {
            printf(";");
        } else {
            printf("%c", ns[i]);
        }
        i++;
    }
    printf("\n");
}

/* adds to toc entry if new name, otherwise overwrites existing val */
void define_string(const char *name, const char *val) {
    /* find existing tocentry, or null if not present */
    tocentry *te = tochead;
    while(te != NULL) {
        /* note: NULL is a valid name */
        if((te->name == NULL && name == NULL) ||
            strncmp(te->name, name, MAX_TOC_NAME_SIZE) == 0) {
            break;
        }
        te = te->next;
    }
    /* if we found an existing entry, overwrite it */
    if(te != NULL) {
        /* free existing val */
        free((void*)te->val);
        /* set new val */
        te->val = (char*)malloc(strlen(val));
        strcpy(te->val, val);
    } else {
        /* new entry (insert at front) */
        te = (tocentry*)malloc(sizeof(tocentry));
        te->next = tochead;
        te->name = (char*)malloc(strlen(name));
        strcpy(te->name, name);
        te->val = (char*)malloc(strlen(val));
        strcpy(te->val, val);
        tochead = te;
    }
}

/* get val for an existing tocentry; if entry doesn't exist, return null */
const char *get_string(const char *name) {
    tocentry *te = tochead;
    while(te != NULL) {
        /* note: NULL is a valid name */
        if((te->name == NULL && name == NULL) ||
            strncmp(te->name, name, MAX_TOC_NAME_SIZE) == 0) {
            break;
        }
        te = te->next;
    }
    if(te != NULL) {
        return te->val;
    } else {
        return NULL;
    }
}

void free_toc() {
    tocentry *next;
    while(tochead != NULL) {
        next = tochead->next;
        free((void*)tochead);
        tochead = next;
    }
}

void debug_print_toc() {
    tocentry *te = tochead;
    printf("Table of contents:\n");
    int i = 0;
    while(te != NULL) {
        printf("  %s => %s\n", te->name, te->val);
        te = te->next;
        i++;
    }
    printf("TOC has %d elements.\n", i);
}

/* #(ds, foo, bar) */
const char *eval_define_string(const char *name, const char *val) {
    //printf("define string: %s => %s\n", name, val);
    define_string(name, val);
    return NULL;
}

/* #(cl, foo) */
const char *eval_call_string(const char *name) {
    //printf("call string: %s\n", name);
    return get_string(name);
}

/* #(rs) */
const char *eval_read_string() {
    /* printf("read string\n"); */
    char *s = NULL;
    size_t slen;
    ssize_t nchars = getdelim(&s, &slen, READ_STRING_DELIM, stdin);
    /* printf("got (%d): %s\n", (int)nchars, s); */
    if(nchars == -1) {
        return (const char*)-1;
    }
    /* remove delimiter */
    s[nchars-1] = 0;
    return s;
}

/* #(ps, foo) */
const char *eval_print_string(const char *s) {
    /* printf("print string: %s\n", s); */
    printf("%s\n", s);
    return NULL;
}

/* creates an array of argptrs (const char *), for funcname and args,
 * plus an extra NULL ptr to indicate end */
char **find_args(char *ns, int start, int end) {
    int pos = start;
    int argcount = 0;
    while(ns[pos] != FUNCEND) {
        if(ns[pos] == ARGPTR) {
            argcount++;
        }
        pos++;
    }
    argcount++; /* account for func name */
    char **argptrs = (char**)malloc((argcount+1)*sizeof(char*));
    argptrs[argcount] = 0; /* NULL terminating ptr */
    pos = start;
    int lastpos = start;
    for(int i = 0; i < argcount; i++) {
        /* move to end of arg */
        while(ns[pos] != FUNCEND && ns[pos] != ARGPTR) pos++;
        ns[pos] = 0; /* nul-terminate arg, for strcpy */
        /* copy arg to argptrs */
        argptrs[i] = (char*)malloc(pos-lastpos);
        strcpy(argptrs[i], ns+lastpos);
        pos++;
        lastpos = pos;
    }
    return argptrs;
}

void free_args(char **argptrs) {
    int i = 0;
    while(argptrs[i] != NULL) {
        free((void*)argptrs[i]);
        i++;
    }
    free((void*)argptrs);
}

void print_args(char **argptrs) {
    int i = 0;
    while(argptrs[i] != NULL) {
        printf("arg %d: \"%s\"\n", i, argptrs[i]);
        i++;
    }
}

/* return -1 to indicate eval() should quit, e.g. eval_read_string reads EOL/EOT */
const char *func_dispatch(char *ns, int start, int end)
{
    const char *rval = NULL;
    char **argptrs = find_args(ns, start, end);

    //print_args(argptrs);

    /* figure out which function is being called */
    if(strncmp(argptrs[0], "rs", MAX_STRING_SIZE) == 0) {
        rval = eval_read_string();
    }
    else if(strncmp(argptrs[0], "ps", MAX_STRING_SIZE) == 0) {
        rval = eval_print_string(argptrs[1]);
    }
    else if(strncmp(argptrs[0], "cl", MAX_STRING_SIZE) == 0) {
        rval = eval_call_string(argptrs[1]);
    }
    else if(strncmp(argptrs[0], "ds", MAX_STRING_SIZE) == 0) {
        rval = eval_define_string(argptrs[1], argptrs[2]);
    }
    else
    {
        printf("unknown function: \"%s\"\n", argptrs[0]);
        rval = NULL;
    }
    free_args(argptrs);
    return rval;
}

/* evaluate an input string (which may cause recursive eval),
 * return resulting string or null; this func will call helper
 * funcs above, e.g., eval_call(); algorithm follows from mooers1966trac */
const char *eval(char *s) {
    int slen = strnlen(s, MAX_STRING_SIZE);
    /* s is the "active string" */
    int sp = 0; /* sp is the "scanning pointer" */
    char *ns = (char*)calloc(MAX_STRING_SIZE, 1); /* neutral string */
    int cl = 0; /* cl is the "current location" (in the neutral string) */
    const char *fval = NULL; /* function return value, for #() functions */
    int funcstart = -1; /* start of current func in ns (ACTIVEFUNC or NEUTRALFUNC mark) */

    rule1:
    /* The character under the scanning pointer is examined. If there is
     * no character left (active string empty), go to rule 14 */
    if(sp >= slen) goto rule14;

    if(s[sp] == 0x04) { /* Ctrl-D */
        return NULL;
    }

    /*
    printf("-s:  ");
    print_ns(s);
    printf("-ns: ");
    print_ns(ns);
    debug_print_toc();
    */

    /*rule2:*/
    /* If the character just examined (by rule 1) is a begin parenthesis,
     * the character is deleted and the pointer is moved ahead to the
     * character following the first matching end parenthesis. The end
     * parenthesis is deleted and all nondeleted characters passed over
     * (including nested parentheses) are put into the neutral string
     * without change. Go to rule 1. */
    if(s[sp] == '(') {
        s[sp] = DELETED;
        /* find matching end parenthesis */
        int parendepth = 0;
        while(++sp < slen) {
            if(s[sp] == ')' && parendepth == 0) {
                s[sp] = DELETED;
                sp++;
                break;
            } else if(s[sp] == ')') {
                parendepth--;
            } else if(s[sp] == '(') {
                parendepth++;
            }
            if(s[sp] != DELETED) {
                ns[cl] = s[sp];
                cl++;
            }
        }
        goto rule1;
    }

    /*rule3:*/
    /* If the character just examined is either a carriage return, a
     * line feed or a tabulate, the character is deleted. Go to rule 15. */
    if(s[sp] == '\r' || s[sp] == '\n' || s[sp] == '\t') {
        s[sp] = DELETED;
        goto rule15;
    }

    /*rule4:*/
    /* If the character just examined is a comma, it is deleted. The
     * location following the right-hand character at the end of the
     * neutral string, called the "current location", is marked by
     * a pointer to indicate the end of an argument substring and the
     * beginning of a new argument substring. Go to rule 15. */
    if(s[sp] == ',') {
        s[sp] = DELETED;
        ns[cl] = ARGPTR;
        cl++;
        goto rule15;
    }

    /*rule5:*/
    /* If the character is a sharp sign, the next character is inspected. If
     * this is a begin parenthesis, the beginning of an active function is
     * indicated. The sharp sign and begin parenthesis are deleted and the
     * current location in the neutral string is marked to indicate the
     * beginning of an active function and the beginning of an argument sub-
     * string. The scanning pointer is moved to the character following the
     * deleted parenthesis. Go to rule 1. */
    if(s[sp] == '#') {
        if(s[sp+1] == '(') {
            s[sp] = DELETED;
            s[sp+1] = DELETED;
            ns[cl] = ACTIVEFUNC; 
            cl++;
            sp = sp+2;
            goto rule1;
        }
    }

    /*rule6:*/
    /* If the character is a sharp sign and the next character is also a sharp
     * sign, the second-following character is inspected. If this is a begin
     * parenthesis, the beginning of a neutral function is indicated. Two sharp
     * signs and the begin parenthesis are deleted and the current location in
     * the neutral string is marked to indicate the beginning of a neutral
     * function and the beginning of an argument sub-string. The scanning
     * pointer is moved to the character following the deleted parenthesis. Go
     * to rule 1. */
    if(s[sp] == '#') {
        if(s[sp+1] == '#') {
            if(s[sp+2] == '(') {
                s[sp] = DELETED;
                s[sp+1] = DELETED;
                s[sp+2] = DELETED;
                ns[cl] = NEUTRALFUNC;
                cl++;
                sp = sp+3;
                goto rule1;
            }
        }
    }

    /*rule7:*/
    /* If the character is a sharp sign, but neither rule 5 or 6 applies, the
     * character is added to the neutral string. Go to rule 15. */
    if(s[sp] == '#' && s[sp+1] != '#' && s[sp+1] != '(') {
        ns[cl] = s[sp];
        cl++;
        goto rule15;
    }

    /*rule8:*/
    /* If the character is an end parenthesis, the character is deleted. The
     * current location in the neutral string is marked by a pointer to
     * indicate the end of an argument substring and the end of a function. The
     * pointer to the beginning of the current function is now retrieved. The
     * complete set of argument substrings for the function have now been
     * defined. The action indicated for the function is performed. Go to
     * rule 10. */
    if(s[sp] == ')') {
        s[sp] = DELETED;
        ns[cl] = FUNCEND;

        //printf("ns: ");
        //print_ns(ns);

        /* find start of current function by walking backwards until we find
         * an ACTIVEFUNC or NEUTRALFUNC mark; save this position */
        funcstart = cl;
        while(funcstart >= 0 && ns[funcstart] != ACTIVEFUNC
            && ns[funcstart] != NEUTRALFUNC) { funcstart--; }

        /* call a helper function to figure out which function is being called
         * and extract the arguments; returns the called function's value */
        //printf("func dispatch: %d, %d\n", funcstart+1, cl);
        //printf("ns: ");
        //print_ns(ns);
        fval = func_dispatch(ns, funcstart+1, cl);
        //printf("got fval: %s\n", fval);
        if(fval == (const char*)-1) {
            return fval;
        }

        goto rule10;
    }

    /*rule9:*/
    /* If the character meets the test of none of the rules 2 through 8,
     * transfer the character to the right-hand end of the neutral string and
     * go to rule 15. */
    /* If we get here, none of the prior rules matched, because they would all
     * have jumped over this part. */
    ns[cl] = s[sp];
    cl++;
    goto rule15;

    rule10:
    /* If the function has null value, go to rule 13. */
    if(fval == NULL) goto rule13;

    /*rule11:*/
    /* If the function was an active function, the value string is inserted to
     * the left of (preceding) the first unscanned character in the active
     * string. The scanning pointer is reset so as to point to the location
     * preceding the first character of the new value string. Go to rule 13. */
    if(ns[funcstart] == ACTIVEFUNC) {
        //printf("got fval: %s\n", fval);
        char *s2;
        //s[sp] = 0;
        asprintf(&s2, "%s%s", fval, s+sp+1);
        free(s);
        s = s2;
        slen = strnlen(s2, MAX_STRING_SIZE);
        sp = -1;
        //printf("new s: ");
        //print_ns(s);
        goto rule13;
    }

    /*rule12:*/
    /* If the function was a neutral function, the value string is inserted in
     * the neutral string with its first character being put in the location
     * pointed to by the current begin-of-function pointer. Delete the argument
     * and function pointers back to the begin-of-function pointer. The
     * scanning pointer is not reset. Go to rule 15. */
    if(ns[funcstart] == NEUTRALFUNC) {
        char *ns2;
        ns[funcstart] = 0; /* terminate neutral string at start of function */
        /* gen new netural string as prior-truncated-neutral + fval */
        asprintf(&ns2, "%s%s", ns, fval);
        cl = strlen(ns) + strlen(fval);
        free(ns);
        ns = ns2;
        //printf("ns rule12: ");
        //print_ns(ns);
        //printf("cl: %d\n", cl);
        goto rule15;
    }

    rule13:
    /* Delete the argument and function pointers back to the begin-of-function
     * pointer for the function just evaluated, resetting the current location
     * to this point. Go to rule 15. */
    /* No need to actually delete function pointers (which we have as just marks
     * in the neutral string); we only need to reset the current location */
    cl = funcstart;
    goto rule15;

    rule14:
    /* Delete the neutral string, initialize its pointers, reload a new copy of
     * the idling procedure into the active string, reset the scanning pointer
     * to the beginning of the idling procedure, and go to rule 1. */

    /* Ignore above, we have our own technique for eval */

    free(ns);
    free(s);
    return NULL;

    rule15:
    /* Move the scanning pointer ahead to the next character. Go to rule 1. */
    /* printf("sp %d (%c) moving to %d (%c)\n", sp, s[sp], sp+1, s[sp+1]); */
    sp++;
    goto rule1;
}

