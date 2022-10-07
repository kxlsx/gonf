#ifndef _PARSE_H
/* ================= */
#define _PARSE_H 1

#ifndef _COMMON_H
#include <common.h>
#endif
#ifndef _FLAGSPEC_H
#include <flagspec.h>
#endif
#ifndef _INFILES_H
#include <infiles.h>
#endif

/* Return values. */
#define PARSEGONF_OK        OK
#define PARSEGONF_ERR_NOMEM ERR_NOMEM
#define PARSEGONF_ERR_PARSE ERR_PARSE

/* Try to parse the given gonf spec files
 * into the passed flagspec structure.
 * 
 * RETURNS:
 *  PARSEGONF_OK
 *  or
 *  PARSEGONF_ERR_*
 *
 * ERRORS:
 *  On error, the function will print an error message
 *  to stderr and continue parsing until EOF, upon which
 *  it will return PARSEGONF_ERR_PARSE.
 *  When a malloc failure occurs, the function will terminate
 *  immediately with PARSEGONF_ERR_NOMEM.
 */
int parsegonf(struct infiles *infiles, struct flagspec *spec);

/* ================= */
#endif