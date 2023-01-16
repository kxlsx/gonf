#ifndef _PARSE_H
/* ================= */
#define _PARSE_H 1

#ifndef _COMMON_H
#include <common.h>
#endif
#ifndef _FLAGSPEC_H
#include <flagspec.h>
#endif
#ifndef _FILES_H
#include <files.h>
#endif

/* Try to parse the given gonf spec files
 * into the passed flagspec structure.
 * 
 * RETURNS:
 *  OK
 *  or
 *  ERR_PARSE | ERR_NOMEM
 *
 * ERRORS:
 *  On error, the function will print an error message
 *  to stderr and continue parsing until EOF, upon which
 *  it will return ERR_PARSE.
 *  When a malloc failure occurs, the function will terminate
 *  immediately with ERR_NOMEM.
 */
int parsegonf(struct filearr *infiles, struct flagspec *spec);

/* ================= */
#endif