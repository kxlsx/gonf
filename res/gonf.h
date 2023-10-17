#ifndef _$P_H
/* ================= */
#if !(defined(_STDBOOL_H) || defined(_INC_STDBOOL))
#include <stdbool.h>
#endif

#define _$P_H 1

/* Possible $perr values */
#define $POK              0
#define $PERR_NOMEM       1
#define $PERR_NOFLAG      2
#define $PERR_UNKNFLAG    3
#define $PERR_NOTVALFLAG  4
#define $PERR_NOVAL       5

/* Value for when a flag has no shortname */
#define $PSHORT_NONE '\0'

typedef int $pc_t;$s

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
struct $plag{
    char *default_value;
    char *value;
    const char *const description;
    const char *const longname;
    $pc_t count;
    const char shortname;
    const bool is_value;
};

/* Macros used to find a certain flag
 * and check whether it appeared.
 */
#define $plag_is_present(INDEX) \
    ($plag_get(INDEX)->count > 0)
#define $plag_is_present_by_short(SHORTNAME) \
    ($plag_get_by_short(SHORTNAME)->count > 0)
#define $plag_is_present_by_long(LONGNAME) \
    ($plag_get_by_long(LONGNAME)->count > 0)

/* Macros used to find a certain flag
 * and return the value of a specific field.
 */
#define $plag_get_default_value(INDEX) \
    ($plag_get(INDEX)->default_value)
#define $plag_get_default_value_by_short(SHORTNAME) \
    ($plag_get_by_short(SHORTNAME)->default_value)
#define $plag_get_default_value_by_long(LONGNAME) \
    ($plag_get_by_long(LONGNAME)->default_value)

#define $plag_get_value(INDEX) \
    ($plag_get(INDEX)->value)
#define $plag_get_value_by_short(SHORTNAME) \
    ($plag_get_by_short(SHORTNAME)->value)
#define $plag_get_value_by_long(LONGNAME) \
    ($plag_get_by_long(LONGNAME)->value)

#define $plag_get_description(INDEX) \
    ($plag_get(INDEX)->description)
#define $plag_get_description_by_short(SHORTNAME) \
    ($plag_get_by_short(SHORTNAME)->description)
#define $plag_get_description_by_long(LONGNAME) \
    ($plag_get_by_long(LONGNAME)->description)

#define $plag_get_longname(INDEX) \
    ($plag_get(INDEX)->longname)
#define $plag_get_longname_by_short(SHORTNAME) \
    ($plag_get_by_short(SHORTNAME)->longname)
#define $plag_get_longname_by_long(LONGNAME) \
    ($plag_get_by_long(LONGNAME)->longname)

#define $plag_get_count(INDEX) \
    ($plag_get(INDEX)->count)
#define $plag_get_count_by_short(SHORTNAME) \
    ($plag_get_by_short(SHORTNAME)->count)
#define $plag_get_count_by_long(LONGNAME) \
    ($plag_get_by_long(LONGNAME)->count)

#define $plag_get_shortname(INDEX) \
    ($plag_get(INDEX)->shortname)
#define $plag_get_shortname_by_short(SHORTNAME) \
    ($plag_get_by_short(SHORTNAME)->shortname)
#define $plag_get_shortname_by_long(LONGNAME) \
    ($plag_get_by_long(LONGNAME)->shortname)

#define $plag_get_is_value(INDEX) \
    ($plag_get(INDEX)->is_value)
#define $plag_get_is_value_by_short(SHORTNAME) \
    ($plag_get_by_short(SHORTNAME)->is_value)
#define $plag_get_is_value_by_long(LONGNAME) \
    ($plag_get_by_long(LONGNAME)->is_value)

/* Return the flag associated with the given index. 
 * Returns NULL on error.
 */
struct $plag *$plag_get($pc_t flag_index);
/* Return the flag associated with the given shortname.
 * Returns NULL on error.
 */
struct $plag *$plag_get_by_short(char shortname);
/* Return the flag associated with the given longname.
 * Returns NULL on error.
 */
struct $plag *$plag_get_by_long(const char *longname);

/* Parse the flags in argv and return 
 * an array of non-flag args.
 * In order to access the parsed flags,
 * use the $plag_get* functions.
 * On succesful termination the $perr variable
 * (accesible with the $perrror* functions)
 * will be set to $POK. Otherwise it will be 
 * set to a $PERR_* value.
 * 
 * RETURNS:
 *  A mallocd array containing 
 *  non-flag args found while parsing.
 *  The length of the array can be accesed 
 *  by using the $pargc function.
 *  or
 *  NULL
 *  
 * ERRORS:
 *  On error, the function will terminate parsing
 *  immediately, return NULL and 
 *  set the $perr variable to an appropriate value.
 *  In order to access the error value,
 *  use the $perror* functions.
 * 
 *  Bear in mind that the contents of the
 *  parsed flags may not be completely correct
 *  after a failed parse. If you want to use them
 *  anyway, read the error number with $perror
 *  and proceed accordingly.
 */
char **$pparse($pc_t argc, char **argv);

/* Return the number of arguments returned from $pparse */
$pc_t $pargc(void);

/* Return the result of the last call to $pparse.
 *
 * RETURNS:
 *  $POK
 *  or
 *  $PERR_*
 */
int $perror(void);
/* Return the value associated with the encountered error.
 * 
 * RETURNS:
 *  NULL if $perror() is set as $POK or $PERR_NOMEM.
 *  or
 *  The token which caused the error (as a pointer to a string in argv).
 */
char *$perror_value(void);
/* Print an error message to stderr if $perror() != $POK.
 *
 * ERROR MESSAGES:
 *  $PERR_NOMEM:
 *   "failed to allocate memory."
 *  $PERR_NOFLAG:
 *   "no flag name provided after 'FLAG'."
 *  $PERR_UNKNFLAG:
 *   "unrecognized flag 'FLAG'."
 *  $PERR_NOTVALFLAG:
 *   "flag 'FLAG' does not take a value."
 *  $PERR_NOVAL:
 *   "no value provided for 'FLAG'."
 *  FLAG is replaced by the token that
 *  caused the error.
 *  Every message is terminated with a newline.
 *
 * RETURNS:
 *  The value of $perror();
 */
int $perror_print(void);

/* ================= */
#endif