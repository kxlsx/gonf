#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define GONFOK             0
#define GONFERR_NOMEM      1
#define GONFERR_NOFLAG     2
#define GONFERR_UNKNFLAG   3
#define GONFERR_NOTVALFLAG 4
#define GONFERR_NOVAL      5

#define GONFSHORT_NULL '\0'

typedef int gonfc_t;
typedef int gonfsize_t;

struct gonflag{
    char *default_value;
    char *value;
    const char *const description;
    const char *const longname;
    gonfc_t count;
    const char shortname;
    const bool is_value;
};

struct gonf_matchlist{
    const gonfc_t index;
    const char *const match;
    struct gonf_matchlist *next;
};