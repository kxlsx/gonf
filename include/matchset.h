#ifndef _MATCHSET_H
/* ================= */
#define _MATCHSET_H 1

#ifndef _COMMON_H
#include <common.h>
#endif

/* matchset return values */
#define MATCHSET_OK     OK
#define MATCHSET_NOMEM  ERR_NOMEM
#define MATCHSET_EXISTS 2

/* List of items having the same hash in the matchset. */
struct matchset_node{
    char *item;
    struct matchset_node *next;
};

/* Struct representing a hashset of string values. */
struct matchset{
    struct matchset_node *stor;
    gonfsize_t len;
    gonfsize_t stor_size;
};

/* Allocate a new, empty matchset */
struct matchset *matchset_new();
/* Free all memory associated with the given matchset. */
void matchset_free(struct matchset *self);

/* Try to insert a string into the matchset.
 *
 * RETURNS:
 *  MATCHSET_OK
 *  or
 *  MATCHSET EXISTS - if the string exists in the matchset
 *  or
 *  MATCHSET NOMEM
 */
int matchset_insert(struct matchset *self, char *item);

/* Check whether the given string exists in the matchset.
 *
 * RETURNS:
 *  a boolean value.
 */
bool matchset_contains(struct matchset *self, char *item);

/* ================= */
#endif