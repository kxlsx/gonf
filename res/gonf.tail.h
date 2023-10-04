
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
 * and return the value of a specific field.
 */
#define gonflag_get_default_value(INDEX) \
    (gonflag_get(INDEX)->default_value)
#define gonflag_get_default_value_by_short(SHORTNAME) \
    (gonflag_get_by_short(SHORTNAME)->default_value)
#define gonflag_get_default_value_by_long(LONGNAME) \
    (gonflag_get_by_long(LONGNAME)->default_value)

#define gonflag_get_value(INDEX) \
    (gonflag_get(INDEX)->value)
#define gonflag_get_value_by_short(SHORTNAME) \
    (gonflag_get_by_short(SHORTNAME)->value)
#define gonflag_get_value_by_long(LONGNAME) \
    (gonflag_get_by_long(LONGNAME)->value)

#define gonflag_get_description(INDEX) \
    (gonflag_get(INDEX)->description)
#define gonflag_get_description_by_short(SHORTNAME) \
    (gonflag_get_by_short(SHORTNAME)->description)
#define gonflag_get_description_by_long(LONGNAME) \
    (gonflag_get_by_long(LONGNAME)->description)

#define gonflag_get_longname(INDEX) \
    (gonflag_get(INDEX)->longname)
#define gonflag_get_longname_by_short(SHORTNAME) \
    (gonflag_get_by_short(SHORTNAME)->longname)
#define gonflag_get_longname_by_long(LONGNAME) \
    (gonflag_get_by_long(LONGNAME)->longname)

#define gonflag_get_count(INDEX) \
    (gonflag_get(INDEX)->count)
#define gonflag_get_count_by_short(SHORTNAME) \
    (gonflag_get_by_short(SHORTNAME)->count)
#define gonflag_get_count_by_long(LONGNAME) \
    (gonflag_get_by_long(LONGNAME)->count)

#define gonflag_get_shortname(INDEX) \
    (gonflag_get(INDEX)->shortname)
#define gonflag_get_shortname_by_short(SHORTNAME) \
    (gonflag_get_by_short(SHORTNAME)->shortname)
#define gonflag_get_shortname_by_long(LONGNAME) \
    (gonflag_get_by_long(LONGNAME)->shortname)

#define gonflag_get_is_value(INDEX) \
    (gonflag_get(INDEX)->is_value)
#define gonflag_get_is_value_by_short(SHORTNAME) \
    (gonflag_get_by_short(SHORTNAME)->is_value)
#define gonflag_get_is_value_by_long(LONGNAME) \
    (gonflag_get_by_long(LONGNAME)->is_value)

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
 *  A mallocd array containing 
 *  non-flag args found while parsing.
 *  The length of the array can be accesed 
 *  by using the gonfargc function.
 *  or
 *  NULL
 *  
 * ERRORS:
 *  On error, the function will terminate parsing
 *  immediately, return NULL and 
 *  set the gonferr variable to an appropriate value.
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
gonfc_t gonfargc(void);

/* Return the result of the last call to gonfparse.
 *
 * RETURNS:
 *  GONFOK
 *  or
 *  GONFERR_*
 */
int gonferror(void);
/* Return the value associated with the encountered error.
 * 
 * RETURNS:
 *  NULL if gonferror() is set as GONFOK or GONFERR_NOMEM.
 *  or
 *  The token which caused the error (as a pointer to a string in argv).
 */
char *gonferror_value(void);
/* Print an error message to stderr if gonferror() != GONFOK.
 *
 * ERROR MESSAGES:
 *  GONFERR_NOMEM:
 *   "failed to allocate memory."
 *  GONFERR_NOFLAG:
 *   "no flag name provided after 'FLAG'."
 *  GONFERR_UNKNFLAG:
 *   "unrecognized flag 'FLAG'."
 *  GONFERR_NOTVALFLAG:
 *   "flag 'FLAG' does not take a value."
 *  GONFERR_NOVAL:
 *   "no value provided for 'FLAG'."
 *  FLAG is replaced by the token that
 *  caused the error.
 *  Every message is terminated with a newline.
 *
 * RETURNS:
 *  The value of gonferror();
 */
int gonferror_print(void);

/* ================= */
#endif