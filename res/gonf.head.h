#ifndef _GONF_H
/* ================= */
#if !(defined(_STDBOOL_H) || defined(_INC_STDBOOL))
#include <stdbool.h>
#endif

#define _GONF_H 1

/* Possible gonferr values */
#define GONFOK              0
#define GONFERR_NOMEM       1
#define GONFERR_NOFLAG      2
#define GONFERR_UNKNFLAG    3
#define GONFERR_NOTVALFLAG  4
#define GONFERR_NOVAL       5

/* Value for when a flag has no shortname */
#define GONFSHORT_NONE '\0'

typedef int gonfc_t;
