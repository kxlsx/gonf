#ifndef _FLAGSPEC_H
/* ================= */
#define _FLAGSPEC_H 1

#ifndef _COMMON_H
#include <common.h>
#endif
#ifndef _MATCHLIST_H
#include <matchlist.h>
#endif

/* flagspec return values. */
#define FLAGSPEC_OK    0
#define FLAGSPEC_NOMEM 1
#define FLAGSPEC_EXIST 2
#define FLAGSPEC_FILLD 3

typedef gonfsize_t flagc_t;

/* The number of possible shortnames 
 * and the ASCII codepoint they start at.
 */
#define FLAGSHORT_MAX 94
#define FLAGSHORT_OFF 33
/* Empty shortname value. */
#define FLAGSHORT_NULL '\0'

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

/* A pool to allocate flagspec strings into */
struct strpool{
    char *stor;
    gonfsize_t size;
    gonfsize_t len;
};

/* Struct containing the collected
 * metadata about a set of flags.
 */
struct flagspec{
    struct flaginfo *stor;
    struct matchlist *rec_longname;
    struct matchlist *rec_identifier;
    struct strpool *strpool_pool;
    gonfsize_t strpool_pool_size;
    gonfsize_t strpool_pool_last;
    flagc_t size;
    flagc_t last;
    flagc_t rec_shortname[FLAGSHORT_MAX];
};

/* Get the flaginfo stored at the given index. */
#define flagspec_at(SPEC, AT) \
    SPEC->stor[AT]
/* Get the number of flags described in the given flagspec. */
#define flagspec_len(SPEC) \
    SPEC->last

/* Allocate a new, empty flagspec. */
struct flagspec *flagspec_new(void);
/* Free all the memory associated with the given flagspec. */
void flagspec_free(struct flagspec *spec);

/* Change flagspec's current flag to the next one. 
 * 
 * RETURNS:
 *  FLAGSPEC_OK 
 *  or
 *  FLAGSPEC_NOMEM
 */
int flagspec_next(struct flagspec *spec);

/* Set the corresponding field at the last flaginfo in the flagspec.
 * 
 * RETURNS:
 *  if successful - FLAGSPEC_OK 
 *  on error      - FLAGSPEC_EXIST | FLAGSPEC_FILLD | FLAGSPEC_NOMEM
 */
int flagspec_set_longname(struct flagspec *spec, char *longname, gonfsize_t len);
int flagspec_set_identifier(struct flagspec *spec, char *identifier, gonfsize_t len);
int flagspec_set_shortname(struct flagspec *spec, char shortname);
int flagspec_set_description(struct flagspec *spec, char *description, gonfsize_t len);
int flagspec_set_value(struct flagspec *spec, char *value, gonfsize_t len);
int flagspec_set_is_value(struct flagspec *spec, bool is_value);

/* ================= */
#endif