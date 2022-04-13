#ifndef _COMP_H
/* ================= */
#define _COMP_H 1

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _COMMON_H
#include <common.h>
#endif

#define COMPILEGONF_OK          OK
#define COMPILEGONF_ERR_NOMEM   ERR_NOMEM
#define COMPILEGONF_ERR_PARSE   ERR_PARSE
#define COMPILEGONF_ERR_NOFLAGS ERR_NOFLAGS

int compilegonf(FILE **infiles, FILE *outfile, FILE *header_outfile);

/* ================= */
#endif