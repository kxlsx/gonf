#ifndef _FLAGSPEC_H
/* ================= */
#define _FLAGSPEC_H 1

#ifndef _COMMON_H
#include <common.h>
#endif
#ifndef _MATCHSET_H
#include <matchset.h>
#endif

/* flagspec return values. */
#define FLAGSPEC_OK    OK
#define FLAGSPEC_NOMEM ERR_NOMEM
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

/* A dynamic array of strpools */
struct strpool_pool{
    struct strpool *stor;
    gonfsize_t size;
    gonfsize_t len;
};

/* Struct containing the collected
 * metadata about a set of flags.
 */
struct flagspec{
    struct flaginfo *stor;
    struct matchset *longname_record;
    struct matchset *identifier_record;
    struct strpool_pool strpools;
    flagc_t size;
    flagc_t last;
    flagc_t shortname_record[FLAGSHORT_MAX];
};

/* Get the flaginfo stored at the given index. */
#define flagspec_at(SPEC, AT) \
    SPEC->stor[AT]
/* Get the number of flags described in the given flagspec. */
#define flagspec_len(SPEC) \
    SPEC->last

/* Allocate a new, empty flagspec. */
struct flagspec *flagspec_new(void);
/* Free all memory associated with the given flagspec. */
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
 *  FLAGSPEC_OK 
 *  or
 *  FLAGSPEC_EXIST | FLAGSPEC_FILLD | FLAGSPEC_NOMEM - on error
 */
int flagspec_set_longname(struct flagspec *spec, char *longname, gonfsize_t len);
int flagspec_set_identifier(struct flagspec *spec, char *identifier, gonfsize_t len);
int flagspec_set_shortname(struct flagspec *spec, char shortname);
int flagspec_set_description(struct flagspec *spec, char *description, gonfsize_t len);
int flagspec_set_value(struct flagspec *spec, char *value, gonfsize_t len);
int flagspec_set_is_value(struct flagspec *spec, bool is_value);

/* ================= */
#endif