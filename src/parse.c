#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parse.h>
#include <lex.h>
#include <matchlist.h>
#include <common.h>
#include <infiles.h>

/* flagspec eturn values. */
#define FLAGSPEC_OK    0
#define FLAGSPEC_NOMEM 1
#define FLAGSPEC_EXIST 2
#define FLAGSPEC_FILLD 3

#define FLAGSPEC_SIZE_INIT 16

static int flagspec_set_longname(struct flagspec *spec, char *longname){
    if(match_find(longname, spec->longindex) != MATCH_NOTFOUND)
        return FLAGSPEC_EXIST;
    if(spec->stor[spec->last].longname != NULL) 
        return FLAGSPEC_FILLD;

    if(matchlist_append(spec->longindex, longname, spec->last) == MATCHLIST_NOMEM)
        return FLAGSPEC_NOMEM;

    spec->stor[spec->last].longname = longname;
    return FLAGSPEC_OK;
}

static int flagspec_set_identifier(struct flagspec *spec, char *identifier){
    if(match_find(identifier, spec->identindex) != MATCH_NOTFOUND)
        return FLAGSPEC_EXIST;
    if(spec->stor[spec->last].identifier != NULL) 
        return FLAGSPEC_FILLD;

    if(matchlist_append(spec->identindex, identifier, spec->last) == MATCHLIST_NOMEM)
        return FLAGSPEC_NOMEM;

    spec->stor[spec->last].identifier = identifier;
    return FLAGSPEC_OK;
}

static int flagspec_set_shortname(struct flagspec *spec, char shortname){
    if(spec->shortindex[shortname % FLAGSHORT_OFF] > 0)                   
        return FLAGSPEC_EXIST;
    if(spec->stor[spec->last].shortname != FLAGSHORT_NULL) 
        return FLAGSPEC_FILLD;

    spec->shortindex[shortname % FLAGSHORT_OFF] = spec->last + 1;
    spec->stor[spec->last].shortname = shortname;
    return FLAGSPEC_OK;
}

static int flagspec_set_description(struct flagspec *spec, char *description){
    if(spec->stor[spec->last].description != NULL)
        return FLAGSPEC_FILLD;

    spec->stor[spec->last].description = description;
    return FLAGSPEC_OK;
}

static int flagspec_set_value(struct flagspec *spec, char *value){
    spec->stor[spec->last].value = value;
    return FLAGSPEC_OK;
}

static int flagspec_set_is_value(struct flagspec *spec, bool is_value){
    spec->stor[spec->last].is_value = is_value;
    return FLAGSPEC_OK;
}

/* Change flagspec's current flag to the next one */
static int flagspec_next(struct flagspec *spec){
    struct flaginfo *tmp;

    spec->last++;
    if(spec->last == spec->size){
        spec->size *= 2;
        tmp = realloc(spec->stor, spec->size * sizeof(struct flaginfo));
        if(tmp == NULL) return FLAGSPEC_NOMEM;

        spec->stor = tmp;
        memset(spec->stor + spec->last, 0, (spec->size - spec->last) * sizeof(struct flaginfo));
    }
    return FLAGSPEC_OK;
}

struct flagspec *flagspec_new(void){
    struct flagspec *s;
    s = calloc(1, sizeof(struct flagspec));
    if(s == NULL) return NULL;
    
    s->stor = calloc(FLAGSPEC_SIZE_INIT, sizeof(struct flaginfo));
    if(s->stor == NULL) return NULL;

    s->longindex = matchlist_new();
    if(s->longindex == NULL) return NULL;

    s->identindex = matchlist_new();
    if(s->identindex == NULL) return NULL;

    s->size = FLAGSPEC_SIZE_INIT;
    s->last = 0;
    return s;
}

void flagspec_free(struct flagspec *spec){
    struct flaginfo f;
    for(flagc_t i = 0; i < flagspec_len(spec); i++){
        f = flagspec_at(spec, i);
        if(f.identifier != NULL)  free(f.identifier);
        if(f.longname != NULL)    free(f.longname);
        if(f.description != NULL) free(f.description);
        if(f.value != NULL)       free(f.value);
    }
    free(spec->stor);
    matchlist_free(spec->longindex);
    matchlist_free(spec->identindex);
    free(spec);
}

/* States of the parsegonf parser. */
enum parsegonf_state{
    PGF_BEG,
    PGF_NAM,
    PGF_STR,
    PGF_VAL,
    PGF_END,
    PGF_ERR,
    PGF_DIE,
};

/* Formats for each token when printing error messages. */
#define PARSEGONF_ERR_FMT_IDN "%s:"
#define PARSEGONF_ERR_FMT_SHR "-%c"
#define PARSEGONF_ERR_FMT_LNG "--%s"
#define PARSEGONF_ERR_FMT_STR "\"%s\""
#define PARSEGONF_ERR_FMT_C   "%c"

/* Print error, set the state to ERR and continue */
#define PARSEGONF_THROW_ERR(TOKENFMT, TOKEN, FMT, ...) { \
    eprintf_gonf("%s [%d:%d]: \""TOKENFMT "\": " FMT, infilename, lexgonf_lineno, lexgonf_colno, TOKEN, ## __VA_ARGS__); \
    return PGF_ERR; \
}
#define PARSEGONF_THROW_ERR_EXIST(TOKENFMT, TOKEN, FIELD) \
    PARSEGONF_THROW_ERR(TOKENFMT, TOKEN, #FIELD " has already been defined.\n");

#define PARSEGONF_THROW_ERR_FILLD(TOKENFMT, TOKEN, FIELD) \
    PARSEGONF_THROW_ERR(TOKENFMT, TOKEN, "repeated " #FIELD " definition.\n"); \

#define PARSEGONF_THROW_ERR_EXPECT(TOKENFMT, TOKEN, EXPECTED, GOT) \
    PARSEGONF_THROW_ERR(TOKENFMT, TOKEN, EXPECTED " expected, not " GOT ".\n");

#define PARSEGONF_THROW_ERR_IDN(TOKENFMT, TOKEN) \
    PARSEGONF_THROW_ERR(TOKENFMT, TOKEN, "identifier must precede flag's name.\n");

/* Try to set current flag's field to a duplicate of lexgonf_lval.text */
#define PARSEGONF_TRY_SET_TEXT(FIELD, TO_DUP_ERR_FMT) { \
    char *dup; \
    int res; \
    \
    dup = strndup(lexgonf_lval.text, lexgonf_leng); \
    if(dup == NULL) return PGF_DIE; \
    \
    res = flagspec_set_##FIELD(flags, dup); \
    switch(res){ \
    case FLAGSPEC_NOMEM: \
        free(dup); \
        return PGF_DIE; \
    case FLAGSPEC_EXIST: \
        free(dup); \
        PARSEGONF_THROW_ERR_EXIST(TO_DUP_ERR_FMT, lexgonf_lval.text, FIELD) \
    case FLAGSPEC_FILLD: \
        free(dup); \
        PARSEGONF_THROW_ERR_FILLD(TO_DUP_ERR_FMT, lexgonf_lval.text, FIELD) \
    case FLAGSPEC_OK: break; \
    } \
}

/* Set current flag's field to a duplicate of lexgonf_lval.text */
#define PARSEGONF_SET_TEXT(FIELD) { \
    char *dup; \
    dup = strndup(lexgonf_lval.text, lexgonf_leng); \
    if(dup == NULL) return PGF_DIE; \
    \
    flagspec_set_##FIELD(flags, dup); \
}

/* Try to set current flag's field to lexgonf_lval.c */
#define PARSEGONF_TRY_SET_C(FIELD, VALUE_ERR_FMT) { \
    if(flagspec_set_##FIELD(flags, lexgonf_lval.c) != FLAGSPEC_OK) \
        PARSEGONF_THROW_ERR_FILLD(VALUE_ERR_FMT, lexgonf_lval.c, FIELD) \
}

/* Set current flag's field to value */
#define PARSEGONF_SET(FIELD, VALUE) { \
    flagspec_set_##FIELD(flags, VALUE); \
}

/* Try to change the current flag to the next, empty one */
#define PARSEGONF_NEXT { \
    if(flagspec_next(flags) != FLAGSPEC_OK) \
        return PGF_DIE; \
    return PGF_BEG; \
}

/* Define a parsegonf_state_STATE function being a glorified
 * switch statement on the passed token.
 */
#define PARSEGONF_STATE_FN_DEFINE(STATE, ON_IDN, ON_SHR, ON_LNG, ON_SEP, ON_STR, ON_ISV) \
    enum parsegonf_state parsegonf_state_##STATE(enum lexgonf_token token, struct flagspec *flags, char *infilename){ \
        switch(token){ \
        case LGF_IDN: \
            ON_IDN \
        case LGF_SHR: \
            ON_SHR \
        case LGF_LNG: \
            ON_LNG \
        case LGF_SEP: \
            ON_SEP \
        case LGF_STR: \
            ON_STR \
        case LGF_ISV: \
            ON_ISV \
        case LGF_ERR: \
            PARSEGONF_THROW_ERR("%s", lexgonf_lval.text, "unexpected token.\n"); \
        default: break; \
        } \
        return PGF_DIE; \
    }

PARSEGONF_STATE_FN_DEFINE(BEG,
    /* IDN */
    PARSEGONF_TRY_SET_TEXT(identifier, PARSEGONF_ERR_FMT_IDN);
    return PGF_BEG;,
    /* SHR */
    PARSEGONF_TRY_SET_C(shortname, PARSEGONF_ERR_FMT_SHR);
    return PGF_NAM;,
    /* LNG */
    PARSEGONF_TRY_SET_TEXT(longname, PARSEGONF_ERR_FMT_LNG);
    return PGF_NAM;,
    /* SEP */
    PARSEGONF_THROW_ERR_EXPECT(PARSEGONF_ERR_FMT_C, lexgonf_lval.c, "name", "separator");,
    /* STR */
    PARSEGONF_THROW_ERR_EXPECT(PARSEGONF_ERR_FMT_STR, lexgonf_lval.text, "name", "string");,
    /* ISV */
    PARSEGONF_THROW_ERR_EXPECT(PARSEGONF_ERR_FMT_C, lexgonf_lval.c, "name", "value sign");
)

PARSEGONF_STATE_FN_DEFINE(NAM,
    /* IDN */
    PARSEGONF_THROW_ERR_IDN(PARSEGONF_ERR_FMT_IDN, lexgonf_lval.text);,
    /* SHR */
    PARSEGONF_TRY_SET_C(shortname, PARSEGONF_ERR_FMT_SHR);
    return PGF_STR;,
    /* LNG */
    PARSEGONF_TRY_SET_TEXT(longname, PARSEGONF_ERR_FMT_LNG);
    return PGF_STR;,
    /* SEP */
    PARSEGONF_NEXT;,
    /* STR */
    PARSEGONF_SET_TEXT(description);
    return PGF_STR;,
    /* ISV */
    PARSEGONF_SET(is_value, true);
    return PGF_VAL;
)

PARSEGONF_STATE_FN_DEFINE(STR,
    /* IDN */
    PARSEGONF_THROW_ERR_IDN(PARSEGONF_ERR_FMT_IDN, lexgonf_lval.text);,
    /* SHR */
    PARSEGONF_THROW_ERR(PARSEGONF_ERR_FMT_SHR, lexgonf_lval.c, "names have already been defined.\n");,
    /* LNG */
    PARSEGONF_THROW_ERR(PARSEGONF_ERR_FMT_LNG, lexgonf_lval.text, "names have already been defined.\n");,
    /* SEP */
    PARSEGONF_NEXT;,
    /* STR */
    PARSEGONF_TRY_SET_TEXT(description, PARSEGONF_ERR_FMT_STR);
    return PGF_STR;,
    /* ISV */
    PARSEGONF_SET(is_value, true);
    return PGF_VAL;
)

PARSEGONF_STATE_FN_DEFINE(VAL,
    /* IDN */
    PARSEGONF_THROW_ERR_EXPECT(PARSEGONF_ERR_FMT_IDN, lexgonf_lval.text, "default value", "identifier");,
    /* SHR */
    PARSEGONF_THROW_ERR_EXPECT(PARSEGONF_ERR_FMT_SHR, lexgonf_lval.c, "default value", "shortname");,
    /* LNG */
    PARSEGONF_THROW_ERR_EXPECT(PARSEGONF_ERR_FMT_LNG, lexgonf_lval.text, "default value", "longname");,
    /* SEP */
    PARSEGONF_NEXT;,
    /* STR */
    PARSEGONF_SET_TEXT(value);
    return PGF_END;,
    /* ISV */
    PARSEGONF_THROW_ERR_FILLD(PARSEGONF_ERR_FMT_C, lexgonf_lval.c, value);
)

PARSEGONF_STATE_FN_DEFINE(END,
    /* IDN */
    PARSEGONF_THROW_ERR_EXPECT(PARSEGONF_ERR_FMT_IDN, lexgonf_lval.text, "separator", "identifier");,
    /* SHR */
    PARSEGONF_THROW_ERR_EXPECT(PARSEGONF_ERR_FMT_SHR, lexgonf_lval.c, "separator", "shortname");,
    /* LNG */
    PARSEGONF_THROW_ERR_EXPECT(PARSEGONF_ERR_FMT_LNG, lexgonf_lval.text, "separator", "longname");,
    /* SEP */
    PARSEGONF_NEXT;,
    /* STR */
    PARSEGONF_THROW_ERR_EXPECT(PARSEGONF_ERR_FMT_STR, lexgonf_lval.text, "separator", "string");,
    /* ISV */
    PARSEGONF_THROW_ERR_EXPECT(PARSEGONF_ERR_FMT_C, lexgonf_lval.c, "separator", "value sign");
)

int parsegonf(struct infiles *infiles, struct flagspec *flags){
    enum parsegonf_state state;
    enum lexgonf_token token;
    char *inpath;
    bool is_err;

    is_err = false;
    for(gonsize_t i = 0; i < infiles_len(infiles); i++){
        /* pass the current infile to the lexer */
        lexgonf_set_in(infiles_get_file(infiles, i));
        inpath = infiles_get_path(infiles, i);

        /* main parse loop */
        state = PGF_BEG;
        while((token = lexgonf()) != LGF_END){
            switch(state){
            case PGF_BEG: state = parsegonf_state_BEG(token, flags, inpath);  break;
            case PGF_NAM: state = parsegonf_state_NAM(token, flags, inpath);  break;
            case PGF_STR: state = parsegonf_state_STR(token, flags, inpath);  break;
            case PGF_VAL: state = parsegonf_state_VAL(token, flags, inpath);  break;
            case PGF_END: state = parsegonf_state_END(token, flags, inpath);  break;
            case PGF_ERR:
                /* advance until the next SEP or until END */
                while(token != LGF_END && token != LGF_SEP) token = lexgonf();
                
                if(flagspec_next(flags) != FLAGSPEC_OK){
                    lexgonf_free();
                    return PARSEGONF_ERR_NOMEM;
                }
                is_err = true;
                state = PGF_BEG;
                break;
            case PGF_DIE:
                lexgonf_free();
                return PARSEGONF_ERR_NOMEM;
            }
        }
        /* cleanup after parsing a file. */
        switch(state){
        case PGF_BEG: break;
        case PGF_ERR: is_err = true; break;
        case PGF_DIE:
            lexgonf_free();
            return PARSEGONF_ERR_NOMEM;
        default:
            /* dump the last flaginfo */
            if(flagspec_next(flags) != FLAGSPEC_OK){
                lexgonf_free();
                return PARSEGONF_ERR_NOMEM;
            }
            break;
        }
    }
    /* destroy the lexer */
    lexgonf_free();
    
    return (is_err) ? PARSEGONF_ERR_PARSE : PARSEGONF_OK;
}
