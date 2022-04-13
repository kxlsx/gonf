
/* Structure containing the data
 * associated with a flag.
 *
 * FIELDS:
 *  + shortname   
 *  ** a single character name
 *  + longname    
 *  ** multiple character name
 *  + description 
 *  ** a description
 *  + is_value    
 *  ** does the flag contain a value or not
 *  + value       
 *  ** flag's value (char *)
 *  + count       
 *  ** how many times has the flag been parsed
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

/* Return true if the flag has appeared while parsing */
#define gonflag_is_present(GONFLAG) \
    (GONFLAG->count > 0)

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