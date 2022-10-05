#ifndef _INFILES_H
/* ================= */
#define _INFILES_H 1

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _COMMON_H
#include <common.h>
#endif

/* Struct representing an array of files to parse.
 * Contains opened files & their paths.
 */
struct infiles{
    FILE **farr;
    char **parr;
    gonsize_t len;
};

#define infiles_get_file(INFILES, INDEX) \
    (INFILES->farr[INDEX])
#define infiles_get_path(INFILES, INDEX) \
    (INFILES->parr[INDEX])
#define infiles_len(INFILES) \
    (INFILES->len)

/* Close the infiles and free the struct */
void infiles_free(struct infiles *infiles);

/* ================= */
#endif