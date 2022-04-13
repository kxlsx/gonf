#ifndef _LEX_H
/* ================= */
#define _LEX_H 1

#ifndef _COMMON_H
#include <common.h>
#endif

typedef gonsize_t lexsize_t;

enum lexgonf_token{
    LGF_END = 0,
    LGF_IDN,
    LGF_SHR,
    LGF_LNG,
    LGF_SEP,
    LGF_STR,
    LGF_ISV,
    LGF_ERR,
};

/* Current line number in the lexed file */
extern lexsize_t lexgonf_lineno;
/* Current column number in the lexed file */
extern lexsize_t lexgonf_colno;

/* Current token's value */
extern union lexgonf_lval{
    char *text;
    char c;
} lexgonf_lval;
/* Current token's value length */
extern lexsize_t lexgonf_leng;

/* Advance the lexer and return the next token */
enum lexgonf_token lexgonf(void);
/* Destroy the lexer */
void lexgonf_free(void);
/* Set the input file for the lexer. */
void lexgonf_set_in(FILE *in);

/* ================= */
#endif