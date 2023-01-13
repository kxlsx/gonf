#ifndef _COMP_H
/* ================= */
#define _COMP_H 1

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _COMMON_H
#include <common.h>
#endif
#ifndef _INFILES_H
#include <infiles.h>
#endif

/* Return values. */
#define COMPILEGONF_OK          OK
#define COMPILEGONF_ERR_NOMEM   ERR_NOMEM
#define COMPILEGONF_ERR_FILE    ERR_FILE
#define COMPILEGONF_ERR_PARSE   ERR_PARSE
#define COMPILEGONF_ERR_NOFLAGS ERR_NOFLAGS

/* Compile the provided gonf spec files into a c library.
 * The function outputs into a file named outfile_name, 
 * if outfile_name is NULL, it writes to stdout.
 * An optional header_output_name can be passed to create an
 * additional C header file for the library.
 * 
 * RETURNS:
 *  COMPILEGONF_OK
 *  or
 *  COMPILEGONF_ERR_*
 * 
 * ERRORS:
 *  On error, the function will print an error message
 *  to stderr and return a COMPILEGONF_ERR_* value.
 */
int compilegonf(struct infiles *infiles, char *outfile_name, char *header_outfile_name);

/* ================= */
#endif