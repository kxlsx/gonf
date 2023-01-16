#include <stdio.h>

#include <parse.h>
#include <flagspec.h>
#include <lex.h>
#include <files.h>
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
#define ERR_FMT_IDN "%s:"
#define ERR_FMT_SHR "-%c"
#define ERR_FMT_LNG "--%s"
#define ERR_FMT_STR "\"%s\""
#define ERR_FMT_C   "%c"

/* Print error, set the state to ERR and continue */
#define THROW_ERR(TOKENFMT, TOKEN, FMT, ...) { \
    eprintf_gonf("%s [%d:%d]: \""TOKENFMT "\": " FMT, infilename, lexgonf_lineno, lexgonf_colno, TOKEN, ## __VA_ARGS__); \
    return PGF_ERR; \
}
#define THROW_ERR_EXIST(TOKENFMT, TOKEN, FIELD) \
    THROW_ERR(TOKENFMT, TOKEN, #FIELD " already defined in another flag.\n");

#define THROW_ERR_FILLD(TOKENFMT, TOKEN, FIELD) \
    THROW_ERR(TOKENFMT, TOKEN, #FIELD " has already been defined.\n");

#define THROW_ERR_EXPECT(TOKENFMT, TOKEN, EXPECTED, GOT) \
    THROW_ERR(TOKENFMT, TOKEN, "expected " EXPECTED ", not " GOT ".\n");

#define THROW_ERR_IDN(TOKENFMT, TOKEN) \
    THROW_ERR(TOKENFMT, TOKEN, "identifier must precede flag's name.\n");

#define THROW_ERR_NAM(TOKENFMT, TOKEN) \
    THROW_ERR(TOKENFMT, TOKEN, "names have already been defined.\n");

/* Try to set current flag's field to lexgonf_lval.text */
#define PARSEGONF_SET_TEXT(FIELD, ERR_FMT) { \
    int ret; \
    \
    ret = flagspec_set_##FIELD(flags, lexgonf_lval.text, lexgonf_leng); \
    switch(ret){ \
    case ERR_NOMEM: \
        return PGF_DIE; \
    case FLAGSPEC_EXIST: \
        THROW_ERR_EXIST(ERR_FMT, lexgonf_lval.text, FIELD) \
    case FLAGSPEC_FILLD: \
        THROW_ERR_FILLD(ERR_FMT, lexgonf_lval.text, FIELD) \
    case OK: default: break; \
    } \
}

/* Try to set current flag's field to lexgonf_lval.c */
#define PARSEGONF_SET_C(FIELD, ERR_FMT) { \
    int ret; \
    \
    ret = flagspec_set_##FIELD(flags, lexgonf_lval.c); \
    switch(ret){ \
    case ERR_NOMEM: \
        return PGF_DIE; \
    case FLAGSPEC_EXIST: \
        THROW_ERR_EXIST(ERR_FMT, lexgonf_lval.c, FIELD) \
    case FLAGSPEC_FILLD: \
        THROW_ERR_FILLD(ERR_FMT, lexgonf_lval.c, FIELD) \
    case OK: default: break; \
    } \
}

/* Set current flag's field to value */
#define PARSEGONF_SET(FIELD, VALUE) { \
    flagspec_set_##FIELD(flags, VALUE); \
}

/* Try to change the current flag to the next, empty one */
#define PARSEGONF_NEXT { \
    if(flagspec_next(flags) != OK) \
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
            THROW_ERR("%s", lexgonf_lval.text, "unexpected token.\n"); \
        default: break; \
        } \
        return PGF_DIE; \
    }

PARSEGONF_STATE_FN_DEFINE(BEG,
    /* IDN */
    PARSEGONF_SET_TEXT(identifier, ERR_FMT_IDN);
    return PGF_BEG;,
    /* SHR */
    PARSEGONF_SET_C(shortname, ERR_FMT_SHR);
    return PGF_NAM;,
    /* LNG */
    PARSEGONF_SET_TEXT(longname, ERR_FMT_LNG);
    return PGF_NAM;,
    /* SEP */
    THROW_ERR_EXPECT(ERR_FMT_C, lexgonf_lval.c, "name", "separator");,
    /* STR */
    THROW_ERR_EXPECT(ERR_FMT_STR, lexgonf_lval.text, "name", "string");,
    /* ISV */
    THROW_ERR_EXPECT(ERR_FMT_C, lexgonf_lval.c, "name", "value sign");
)

PARSEGONF_STATE_FN_DEFINE(NAM,
    /* IDN */
    THROW_ERR_IDN(ERR_FMT_IDN, lexgonf_lval.text);,
    /* SHR */
    PARSEGONF_SET_C(shortname, ERR_FMT_SHR);
    return PGF_STR;,
    /* LNG */
    PARSEGONF_SET_TEXT(longname, ERR_FMT_LNG);
    return PGF_STR;,
    /* SEP */
    PARSEGONF_NEXT;,
    /* STR */
    PARSEGONF_SET_TEXT(description, ERR_FMT_STR);
    return PGF_STR;,
    /* ISV */
    PARSEGONF_SET(is_value, true);
    return PGF_VAL;
)

PARSEGONF_STATE_FN_DEFINE(STR,
    /* IDN */
    THROW_ERR_IDN(ERR_FMT_IDN, lexgonf_lval.text);,
    /* SHR */
    THROW_ERR_NAM(ERR_FMT_SHR, lexgonf_lval.c);,
    /* LNG */
    THROW_ERR_NAM(ERR_FMT_LNG, lexgonf_lval.text);,
    /* SEP */
    PARSEGONF_NEXT;,
    /* STR */
    PARSEGONF_SET_TEXT(description, ERR_FMT_STR);
    return PGF_STR;,
    /* ISV */
    PARSEGONF_SET(is_value, true);
    return PGF_VAL;
)

PARSEGONF_STATE_FN_DEFINE(VAL,
    /* IDN */
    THROW_ERR_EXPECT(ERR_FMT_IDN, lexgonf_lval.text, "default value", "identifier");,
    /* SHR */
    THROW_ERR_EXPECT(ERR_FMT_SHR, lexgonf_lval.c, "default value", "shortname");,
    /* LNG */
    THROW_ERR_EXPECT(ERR_FMT_LNG, lexgonf_lval.text, "default value", "longname");,
    /* SEP */
    PARSEGONF_NEXT;,
    /* STR */
    PARSEGONF_SET_TEXT(value, ERR_FMT_STR);
    return PGF_END;,
    /* ISV */
    THROW_ERR_FILLD(ERR_FMT_C, lexgonf_lval.c, value);
)

PARSEGONF_STATE_FN_DEFINE(END,
    /* IDN */
    THROW_ERR_EXPECT(ERR_FMT_IDN, lexgonf_lval.text, "separator", "identifier");,
    /* SHR */
    THROW_ERR_EXPECT(ERR_FMT_SHR, lexgonf_lval.c, "separator", "shortname");,
    /* LNG */
    THROW_ERR_EXPECT(ERR_FMT_LNG, lexgonf_lval.text, "separator", "longname");,
    /* SEP */
    PARSEGONF_NEXT;,
    /* STR */
    THROW_ERR_EXPECT(ERR_FMT_STR, lexgonf_lval.text, "separator", "string");,
    /* ISV */
    THROW_ERR_EXPECT(ERR_FMT_C, lexgonf_lval.c, "separator", "value sign");
)

int parsegonf(struct filearr *infiles, struct flagspec *flags){
    enum parsegonf_state state;
    enum lexgonf_token token;
    char *inpath;
    bool is_err;

    is_err = false;
    for(gonfsize_t i = 0; i < infiles->len; i++){
        lexgonf_set_in(infiles->files[i].handle);
        inpath = infiles->files[i].path;

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
                
                if(flagspec_next(flags) != OK){
                    lexgonf_free();
                    return ERR_NOMEM;
                }
                is_err = true;
                state = PGF_BEG;
                break;
            case PGF_DIE:
                lexgonf_free();
                return ERR_NOMEM;
            }
        }
        /* cleanup after parsing a file. */
        switch(state){
        case PGF_BEG: break;
        case PGF_ERR: is_err = true; break;
        case PGF_DIE:
            lexgonf_free();
            return ERR_NOMEM;
        default:
            /* dump the last flaginfo */
            if(flagspec_next(flags) != OK){
                lexgonf_free();
                return ERR_NOMEM;
            }
            break;
        }
    }
    /* destroy the lexer */
    lexgonf_free();
    
    return (is_err) ? ERR_PARSE : OK;
}
