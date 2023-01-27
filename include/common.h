#ifndef _COMMON_H
/* ================= */
#define _COMMON_H 1

#if !(defined(_STDBOOL_H) || defined(_INC_STDBOOL))
#include <stdbool.h>
#endif

typedef int gonfsize_t;

/* Return values (must be <= 15)*/
#define OK          0
#define ERR_NOMEM   1
#define ERR_PARSE   2
#define ERR_FILE    3
#define ERR_NOFLAGS 4
#define ERR_CLI     5

#define DEFAULT_OUTFILE \
    "gonf.c"

#define NAME \
    "gonf"
#define VERSION \
    "0.4.1"
#define DESCRIPTION \
    "Generate programs that parse command-line options."
#define AUTHORS \
    "Łukasz Dragon <lukasz.b.dragon@gmail.com>"
#define LICENSE \
    "This is free software. You may redistribute copies of it under the terms of\n" \
    "the GNU General Public License <https://www.gnu.org/licenses/gpl.html>.\n"     \
    "There is NO WARRANTY, to the extent permitted by law."

/* stringize macro result */
#define XSTR(S) STR(S)
#define STR(S) #S

/* stdio utils */
#if defined(_STDIO_H) || defined(_INC_STDIO)
#define eprintf(FMT, ...) fprintf(stderr, FMT, ## __VA_ARGS__)
#define eprintf_gonf(FMT, ...) eprintf(NAME ": " FMT, ## __VA_ARGS__)
#endif

/* string utils */
#if defined(_STRING_H) || defined(_INC_STRING)
#define streq(A, B) (strcmp(A, B) == 0)
#endif

/* ================= */
#endif
