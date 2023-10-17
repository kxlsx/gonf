#ifndef _COMP_H
/* ================= */
#define _COMP_H 1

#if !(defined(_STDIO_H) || defined(_INC_STDIO))
#include <stdio.h>
#endif

#ifndef _COMMON_H
#include <common.h>
#endif
#ifndef _FILES_H
#include <files.h>
#endif

/* Static variables from the gonf_template.c file. */
/* gonf lib file */
#define gonf_c_template res_gonf_c
#define gonf_c_template_len res_gonf_c_len
extern unsigned int res_gonf_c_len;
extern unsigned char res_gonf_c[];
/* gonf header file */
#define gonf_h_template res_gonf_h
#define gonf_h_template_len res_gonf_h_len
extern unsigned int res_gonf_h_len;
extern unsigned char res_gonf_h[];

/* Compile the provided gonf spec files into a C library.
 * If the fields in header_outfile are not NULL, 
 * an additional C header file is written onto it.
 * A C library and header prefix needs to be specified.
 * 
 * RETURNS:
 *  OK
 *  or
 *  ERR_PARSE | ERR_FILE | ERR_NOFLAGS | ERR_NOMEM
 * 
 * ERRORS:
 *  On error, the function will print an error message
 *  to stderr and return a ERR_* value.
 */
int compilegonf(struct filearr *infiles, struct file outfile, struct file header_outfile,  char *prefix);

/* ================= */
#endif