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

/* Static variables from the gonf_dump.c file. */
/* head & tail of the gonf lib file */
#define gonf_head_c_dump res_gonf_head_c
#define gonf_head_c_dump_len res_gonf_head_c_len
#define gonf_tail_c_dump res_gonf_tail_c
#define gonf_tail_c_dump_len res_gonf_tail_c_len
extern unsigned char res_gonf_head_c[];
extern unsigned int res_gonf_head_c_len;
extern unsigned char res_gonf_tail_c[];
extern unsigned int res_gonf_tail_c_len;
/* head & tail of the gonf header file */
#define gonf_head_h_dump res_gonf_head_h
#define gonf_head_h_dump_len res_gonf_head_h_len
#define gonf_tail_h_dump res_gonf_tail_h
#define gonf_tail_h_dump_len res_gonf_tail_h_len
extern unsigned char res_gonf_head_h[];
extern unsigned int res_gonf_head_h_len;
extern unsigned char res_gonf_tail_h[];
extern unsigned int res_gonf_tail_h_len;

/* Compile the provided gonf spec files into a C library.
 * If the fields in header_outfile are not NULL, 
 * an additional C header file is written onto it.
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
int compilegonf(struct filearr *infiles, struct file outfile, struct file header_outfile);

/* ================= */
#endif