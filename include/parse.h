#ifndef _PARSE_H
/* ================= */
#define _PARSE_H 1

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _MATCHLIST_H
#include <matchlist.h>
#endif
#ifndef _COMMON_H
#include <common.h>
#endif
#ifndef _INFILES_H
#include <infiles.h>
#endif

/* Return values. */
#define PARSEGONF_OK        OK
#define PARSEGONF_ERR_NOMEM ERR_NOMEM
#define PARSEGONF_ERR_PARSE ERR_PARSE

/* The number of possible shortnames 
 * and the ASCII codepoint they start at.
 */
#define FLAGSHORT_MAX 94
#define FLAGSHORT_OFF 33
/* Empty shortname value. */
#define FLAGSHORT_NULL '\0'

typedef gonsize_t flagc_t;

/* Struct containing metadata
 * about a flag.
 */
struct flaginfo{
    char *longname;
    char *identifier;
    char *description;
    char *value;
    bool is_value;
    char shortname;
};

/* Struct containing the collected
 * metadata about a set of flags.
 */
struct flagspec{
    struct flaginfo *stor;
    struct matchlist *longindex;
    struct matchlist *identindex;
    flagc_t size;
    flagc_t last;
    flagc_t shortindex[FLAGSHORT_MAX];
};

/* Allocate a new, empty flagspec. */
struct flagspec *flagspec_new(void);
/* Free all the memory associated with the given flagspec. */
void flagspec_free(struct flagspec *spec);

/* Get the flaginfo stored at the given index. */
#define flagspec_at(spec, at) \
    spec->stor[at]
/* Get the number of flags described in the given flagspec. */
#define flagspec_len(spec) \
    spec->last

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