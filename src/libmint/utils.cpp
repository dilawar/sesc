#include <stdio.h>
#include <stdlib.h>

#include <stdarg.h>

#include "Events.h"

/* The default maximum number of error and warning messages. If these limits
 * are reached, then the program will exit. This prevents long-running
 * programs from generating so many messages that an output file fills
 * up the disk. These limits are stored in the global variables
 * "Max_errors" and "Max_warnings" and can be changed by the back-end,
 * if necessary. If these limits are set to 0 (or any negative number)
 * then the limits are ignored.
 */
#define MAX_ERRORS 100
#define MAX_WARNINGS 1000000

int32_t Max_errors = MAX_ERRORS;
int32_t Max_warnings = MAX_WARNINGS;

/*
 * This routine prints an error message and exits.
 */

void mint_termination(int);

/*VARARGS1*/
void fatal(const char *fmt, ...)
{
    va_list ap;
#if 0
    char c, *s; int32_t d;
#endif

    fflush(stdout);
    fprintf(stderr, "\nERROR: ");
    va_start(ap, fmt);
    vfprintf(stderr,fmt,ap);
    va_end(ap);

    /* should be passed the pid of the thread that generated the fatal, but... */
    mint_termination_err(0,-1); 
}

/*
 * This routine prints an error message.
 *
 * To prevent a long-running program from generating so many error
 * messages that an output file fills up the disk, a limit is placed
 * on how many total error messages can be produced. This limit
 * is a global variable and can be changed by the back-end, if necessary.
 */

/*VARARGS1*/
void error(const char *fmt, ...)
{
    va_list ap;
#if 0
    char c, *s; int32_t d;
#endif
    static int32_t total_errors = 0;

    fflush(stdout);
    fprintf(stderr, "\nERROR: ");
    va_start(ap, fmt);
    vfprintf(stderr,fmt,ap);
    va_end(ap);

    if (Max_errors > 0 && total_errors++ >= Max_errors) {
        fprintf(stderr, "Too many errors (%d)\n", total_errors);
        exit(1);
    }
}

/*
 * This routine prints a warning message.
 *
 * To prevent a long-running program from generating so many warning
 * messages that an output file fills up the disk, a limit is placed
 * on how many total warning messages can be produced. This limit
 * is a global variable and can be changed by the back-end, if necessary.
 */

/*VARARGS1*/
void warning(const char *fmt, ...)
{
    va_list ap;
#if 0
    char c, *s; int32_t d;
#endif
    static int32_t total_warnings = 0;

    fflush(stdout);
    fprintf(stderr, "\nWarning: ");
    va_start(ap, fmt);
    vfprintf(stderr,fmt,ap);
    va_end(ap);

    if (Max_warnings > 0 && total_warnings++ >= Max_warnings) {
        fprintf(stderr, "Too many warnings (%d)\n", total_warnings);
        exit(1);
    }
}

/* strlen_expand() returns the length of the string, counting tabs
 * as the equivalent number of spaces that would be generated.
 */
int32_t strlen_expand(const char *str)
{
    int32_t len;

    if (str == NULL)
        return 0;
    for (len = 0; *str; len++, str++)
        if (*str == '\t')
            len += 7 - (len % 8);
    return len;
}

/*
 * base2roundup() returns the log (base 2) of its argument, rounded up.
 * It also rounds up its argument to the next higher power of 2.
 *
 * Example:
 *   int32_t logsize, size = 5;
 * 
 *   logsize = base2roundup(&size);
 *
 * will make logsize = 3, and size = 8.
 */
int
base2roundup(int32_t *pnum)
{
    int32_t logsize, exp;

    for (logsize = 0, exp = 1; exp < *pnum; logsize++)
        exp *= 2;
    
    /* round pnum up to nearest power of 2 */
    *pnum = exp;

    return logsize;
}

/* returns the number of newlines in "buf"
 */
int32_t newlines(const char *buf)
{
    int32_t count;

    count = 0;
    while (*buf) {
        if (*buf == '\n')
            count++;
        buf++;
    }
    return count;
}
