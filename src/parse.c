#include <stdio.h>

#include <parse.h>
#include <flagspec.h>
#include <lex.h>
#include <infiles.h>
#include <common.h>

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
    PARSEGONF_THROW_ERR(TOKENFMT, TOKEN, #FIELD " already defined in another flag.\n");

#define PARSEGONF_THROW_ERR_FILLD(TOKENFMT, TOKEN, FIELD) \
    PARSEGONF_THROW_ERR(TOKENFMT, TOKEN, #FIELD " has already been defined.\n");

#define PARSEGONF_THROW_ERR_EXPECT(TOKENFMT, TOKEN, EXPECTED, GOT) \
    PARSEGONF_THROW_ERR(TOKENFMT, TOKEN, "expected " EXPECTED ", not " GOT ".\n");

#define PARSEGONF_THROW_ERR_IDN(TOKENFMT, TOKEN) \
    PARSEGONF_THROW_ERR(TOKENFMT, TOKEN, "identifier must precede flag's name.\n");

#define PARSEGONF_THROW_ERR_NAM(TOKENFMT, TOKEN) \
    PARSEGONF_THROW_ERR(TOKENFMT, TOKEN, "names have already been defined.\n");

/* Try to set current flag's field to lexgonf_lval.text */
#define PARSEGONF_SET_TEXT(FIELD, ERR_FMT) { \
    int res; \
    \
    res = flagspec_set_##FIELD(flags, lexgonf_lval.text, lexgonf_leng); \
    switch(res){ \
    case FLAGSPEC_NOMEM: \
        return PGF_DIE; \
    case FLAGSPEC_EXIST: \
        PARSEGONF_THROW_ERR_EXIST(ERR_FMT, lexgonf_lval.text, FIELD) \
    case FLAGSPEC_FILLD: \
        PARSEGONF_THROW_ERR_FILLD(ERR_FMT, lexgonf_lval.text, FIELD) \
    case FLAGSPEC_OK: break; \
    } \
}

/* Try to set current flag's field to lexgonf_lval.c */
#define PARSEGONF_SET_C(FIELD, ERR_FMT) { \
    int res; \
    \
    res = flagspec_set_##FIELD(flags, lexgonf_lval.c); \
    switch(res){ \
    case FLAGSPEC_NOMEM: \
        return PGF_DIE; \
    case FLAGSPEC_EXIST: \
        PARSEGONF_THROW_ERR_EXIST(ERR_FMT, lexgonf_lval.c, FIELD) \
    case FLAGSPEC_FILLD: \
        PARSEGONF_THROW_ERR_FILLD(ERR_FMT, lexgonf_lval.c, FIELD) \
    case FLAGSPEC_OK: break; \
    } \
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
    PARSEGONF_SET_TEXT(identifier, PARSEGONF_ERR_FMT_IDN);
    return PGF_BEG;,
    /* SHR */
    PARSEGONF_SET_C(shortname, PARSEGONF_ERR_FMT_SHR);
    return PGF_NAM;,
    /* LNG */
    PARSEGONF_SET_TEXT(longname, PARSEGONF_ERR_FMT_LNG);
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
    PARSEGONF_SET_C(shortname, PARSEGONF_ERR_FMT_SHR);
    return PGF_STR;,
    /* LNG */
    PARSEGONF_SET_TEXT(longname, PARSEGONF_ERR_FMT_LNG);
    return PGF_STR;,
    /* SEP */
    PARSEGONF_NEXT;,
    /* STR */
    PARSEGONF_SET_TEXT(description, PARSEGONF_ERR_FMT_STR);
    return PGF_STR;,
    /* ISV */
    PARSEGONF_SET(is_value, true);
    return PGF_VAL;
)

PARSEGONF_STATE_FN_DEFINE(STR,
    /* IDN */
    PARSEGONF_THROW_ERR_IDN(PARSEGONF_ERR_FMT_IDN, lexgonf_lval.text);,
    /* SHR */
    PARSEGONF_THROW_ERR_NAM(PARSEGONF_ERR_FMT_SHR, lexgonf_lval.c);,
    /* LNG */
    PARSEGONF_THROW_ERR_NAM(PARSEGONF_ERR_FMT_LNG, lexgonf_lval.text);,
    /* SEP */
    PARSEGONF_NEXT;,
    /* STR */
    PARSEGONF_SET_TEXT(description, PARSEGONF_ERR_FMT_STR);
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
    PARSEGONF_SET_TEXT(value, PARSEGONF_ERR_FMT_STR);
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
    for(gonfsize_t i = 0; i < infiles_len(infiles); i++){
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
