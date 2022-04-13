#ifndef _MATCHLIST_H
/* ================= */
#define _MATCHLIST_H 1

#ifndef _STDDEF_H
#include <stddef.h>
#endif

#ifndef _COMMON_H
#include <common.h>
#endif

#define MATCHLIST_OK     OK
#define MATCHLIST_NOMEM  ERR_NOMEM

#define MATCH_NOTFOUND  -1

typedef gonsize_t matchc_t;

/* A match entry in a matchlist */
struct matchnode{
    matchc_t index;
    char *match;
    struct matchnode *next;
};

/* An easily searchable collection
 * of strings.
 */
struct matchlist{
    struct matchnode *matches;
    matchc_t size;
    matchc_t len;
};


/* Allocate a new, empty matchlist */
struct matchlist *matchlist_new();
/* Free all memory associated with the given matchlist */
void matchlist_free(struct matchlist *list);

/* Add a new string associated with an index to a matchlist */
int matchlist_append(struct matchlist *list, char *match, matchc_t index);

/* Find the first occurence of needle in the given
 * matchlist.
 *
 * RETURNS:
 *  needle's associated index in the matchlist
 *  or
 *  MATCH_NOTFOUND
 */
int match_find(char *needle, struct matchlist *list);

/* ================= */
#endif