#ifndef _COMMON_H
/* ================= */
#define _COMMON_H 1

#ifndef _STDBOOL_H
#include <stdbool.h>
#endif

typedef int gonsize_t;

/* Return values. */
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
    "0.2.0"
#define DESCRIPTION \
    "Generate programs that parse command line flags."
#define AUTHORS \
    "Łukasz Dragon <lukasz.b.dragon@gmail.com>"
#define LICENSE \
    "This is free software. You may redistribute copies of it under the terms of\n" \
    "the GNU General Public License <https://www.gnu.org/licenses/gpl.html>.\n"     \
    "There is NO WARRANTY, to the extent permitted by law."

#define eprintf(FMT, ...) fprintf(stderr, FMT, ## __VA_ARGS__)
#define eprintf_gonf(FMT, ...) eprintf(NAME ": " FMT, ## __VA_ARGS__)

/* stringize macro result */
#define XSTR(S) STR(S)
#define STR(S) #S

/* ================= */
#endif
