#ifndef _GONF_H
/* ================= */
#ifndef _STDBOOL_H
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

#define GONFLAGC 6

#define GONFLAG_INDEX(IDENTIFIER) GONFLAG_##IDENTIFIER
#define GONFLAG_LICENSE 0
#define GONFLAG_VERSION 1
#define GONFLAG_HELP 2
#define GONFLAG_STDOUT 3
#define GONFLAG_OUTPUT 4
#define GONFLAG_HEADER 5

/* Structure containing the data
 * associated with a flag.
 *
 * FIELDS:
 *  + shortname     - a single character name.
 *  + longname      - multiple character name.
 *  + description   - a description.
 *  + is_value      - whether the flag accepts a value or not.
 *  + default_value - flag's default value (char *).
 *  + value         - flag's value (char *).
 *  + count         - how many times has the flag been parsed.
 */
struct gonflag{
    char *default_value;
    char *value;
    const char *const description;
    const char *const longname;
    gonfc_t count;
    const char shortname;
    const bool is_value;
};

/* Macros used to find a certain flag
 * and check whether it appeared.
 */
#define gonflag_is_present(INDEX) \
    (gonflag_get(INDEX)->count > 0)
#define gonflag_is_present_by_short(SHORTNAME) \
    (gonflag_get_by_short(SHORTNAME)->count > 0)
#define gonflag_is_present_by_long(LONGNAME) \
    (gonflag_get_by_long(LONGNAME)->count > 0)

/* Macros used to find a certain flag
 * and return the value of the passed field.
 */
#define gonflag_get_field(INDEX, FIELD) \
    (gonflag_get(INDEX)->FIELD)
#define gonflag_get_field_by_short(SHORTNAME, FIELD) \
    (gonflag_get_by_short(SHORTNAME)->FIELD)
#define gonflag_get_field_by_long(LONGNAME, FIELD) \
    (gonflag_get_by_long(LONGNAME)->FIELD)

/* Return the flag associated with the given index. 
 * Returns NULL on error.
 */
struct gonflag *gonflag_get(gonfc_t flag_index);
/* Return the flag associated with the given shortname.
 * Returns NULL on error.
 */
struct gonflag *gonflag_get_by_short(char shortname);
/* Return the flag associated with the given longname.
 * Returns NULL on error.
 */
struct gonflag *gonflag_get_by_long(const char *longname);

/* Parse the flags in argv and return 
 * an array of non-flag args.
 * In order to access the parsed flags,
 * use the gonflag_get* functions.
 * On succesful termination the gonferr variable
 * (accesible with the gonferrror* functions)
 * will be set to GONFOK. Otherwise it will be 
 * set to a GONFERR_* value.
 * 
 * RETURNS:
 *  A mallocd array terminated by a NULL pointer
 *  containing non-flag args found when parsing.
 *  
 * ERRORS:
 *  On error, the function will terminate parsing
 *  immediately and set the gonferr variable to
 *  an appropriate value.
 *  In order to access the error value,
 *  use the gonferror* functions.
 * 
 *  Bear in mind that the contents of the
 *  parsed flags may not be completely correct
 *  after a failed parse. If you want to use them
 *  anyway, read the error number with gonferror
 *  and proceed accordingly.
 */
char **gonfparse(gonfc_t argc, char **argv);

/* Return the number of arguments returned from gonfparse */
gonfc_t gonfargc(char **gonfargs);

int gonferror(void);
char *gonferror_value(void);
int gonferror_print(void);

/* ================= */
#endif