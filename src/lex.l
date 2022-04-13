%option noinput
%option noyywrap

%{
#include <stdio.h>
#include <string.h>

#include <lex.h>

union lexgonf_lval lexgonf_lval;
lexsize_t lexgonf_leng;

lexsize_t lexgonf_lineno;
lexsize_t lexgonf_colno;
static lexsize_t lexgonf_next_lineno = 1;
static lexsize_t lexgonf_next_colno = 1;

#define LEXGONF_LVAL_SET_TEXT(TEXT, TEXTLEN) \
    lexgonf_lval.text = TEXT; \
    lexgonf_leng = TEXTLEN; \
    lexgonf_lval.text[lexgonf_leng] = '\0';
#define LEXGONF_LVAL_SET_C(C) \
    lexgonf_lval.c = C; \
    lexgonf_leng = 1;

#define LINE_NEXT(OFFSET) { \
    lexgonf_next_lineno += OFFSET; \
    lexgonf_next_colno = 1; \
}
#define COL_NEXT(OFFSET) { \
    lexgonf_next_colno += OFFSET; \
}
#define POS_SAVE { \
    lexgonf_colno = lexgonf_next_colno; \
    lexgonf_lineno = lexgonf_next_lineno; \
}

#define YY_USER_ACTION POS_SAVE; COL_NEXT(yyleng);
%}

SEP ;
ISV =

IDN [[:alpha:]_][[:alnum:]_]*

SHR_PREFIX -
LNG_PREFIX -
SHR [^[:cntrl:] ;=\-"]
LNG {SHR}[^[:cntrl:] ;="]*

STR \"(\\.|[^\0"\\])*\"

%x COMMENT
%x ERROR

%%
\/\*                 BEGIN(COMMENT);
<COMMENT>[^\n*]*
<COMMENT>\*[^*/\n]*
<COMMENT>\n          LINE_NEXT(1);
<COMMENT>\*\/        BEGIN(INITIAL);

\/\/.*\n             LINE_NEXT(1);

\n+         LINE_NEXT(yyleng);
[ \t\r\f]+


{STR} {
    lexsize_t last_lf;

    last_lf = yyleng;
    for(lexsize_t i = 0; i < yyleng; i++){
        if(yytext[i] == '\n') LINE_NEXT(1); last_lf = i;
    }
    COL_NEXT(yyleng - last_lf);

    LEXGONF_LVAL_SET_TEXT(yytext + 1, yyleng - 2);
    return LGF_STR;
}

{IDN}: {
    LEXGONF_LVAL_SET_TEXT(yytext, yyleng - 1);
    return LGF_IDN;
}

{SHR_PREFIX}{SHR} {
    LEXGONF_LVAL_SET_C(yytext[1]);
    return LGF_SHR;
}

{SHR_PREFIX}{LNG_PREFIX}{LNG} {
    LEXGONF_LVAL_SET_TEXT(yytext + 2, yyleng - 2);
    return LGF_LNG;
}

{ISV} {
    LEXGONF_LVAL_SET_C(*yytext); 
    return LGF_ISV;
}

{SEP} {
    LEXGONF_LVAL_SET_C(*yytext);
    return LGF_SEP;
}

. {
    unput(*yytext); 
    BEGIN(ERROR);
}
<ERROR>[[:graph:]]+[\n\t\r\f ] {
    if(yytext[yyleng - 1] == '\n') LINE_NEXT(1);

    LEXGONF_LVAL_SET_TEXT(yytext, yyleng - 1);
    BEGIN(INITIAL);
    return LGF_ERR;
}

<ERROR>.+ {
    LEXGONF_LVAL_SET_TEXT(yytext, yyleng);
    BEGIN(INITIAL);
    return LGF_ERR;
}

<INITIAL,COMMENT><<EOF>> return LGF_END;
%%

enum lexgonf_token lexgonf(void){
    return yylex();
}

void lexgonf_free(void){
    yylex_destroy();
}

void lexgonf_set_in(FILE *in){
    yyin = in;
}
