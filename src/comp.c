#include <stdio.h>

#include <comp.h>
#include <infiles.h>
#include <parse.h>
#include <gonf_dump.h>

static void compile_gonf_identifiers(struct flagspec *flags, FILE *outfile){
    struct matchnode *idents;
    flagc_t idents_count;

    idents_count = flags->identindex->len;
    idents = flags->identindex->matches;
    fputs("#define GONFLAG_INDEX(IDENTIFIER) GONFLAG_##IDENTIFIER\n", outfile);
    for(flagc_t i = 0; i < idents_count; i++){
        fprintf(
            outfile, 
            "#define GONFLAG_%s %d\n", 
            idents[i].match, 
            idents[i].index
        );
    }
}

static void compile_gonflagc(flagc_t flagspec_len, FILE *outfile){
    fprintf(outfile, "\n#define GONFLAGC %d\n\n", flagspec_len);
}

static void compile_header(struct flagspec *flags, FILE *outfile){
    fwrite(gonf_head_h_dump, 1, gonf_head_h_dump_len, outfile);

    compile_gonflagc(flagspec_len(flags), outfile);
    compile_gonf_identifiers(flags, outfile);

    fwrite(gonf_tail_h_dump, 1, gonf_tail_h_dump_len, outfile);
}

static void compile_multiline_str(char *str, FILE *outfile){
    char c;
    putc('\"', outfile);
    while((c = *(str++)) != '\0'){
        switch(c){
        case '\n':   fputs("\\n\"\n\t\"", outfile);  break;
        case '\t':   fputs("\\t", outfile);          break;
        case '\x01'...'\x08':
        case '\x13'...'\x1f': 
        case '\x7f': fprintf(outfile, "\\x%02x", c); break;
        default:     fprintf(outfile, "%c", c);      break;
        }
    }
    fputs("\", ", outfile);
}

static void compile_gonf_flags(struct flagspec *flags, FILE *outfile){
    struct flaginfo flag;

    fputs("static struct gonflag gonf_flags[GONFLAGC] = {\n", outfile);
    for(flagc_t flagi = 0; flagi < flagspec_len(flags); flagi++){
        flag = flagspec_at(flags, flagi);

        fputs("\t{", outfile);
        if(flag.value != NULL)               compile_multiline_str(flag.value, outfile);
        else                                 fputs("NULL, ", outfile);

        fputs("NULL, ", outfile);

        if(flag.description != NULL)         compile_multiline_str(flag.description, outfile);
        else                                 fputs("NULL, ", outfile);

        if(flag.longname != NULL)            fprintf(outfile, "\"%s\", ", flag.longname);
        else                                 fputs("NULL, ", outfile);

        fputs("0, ", outfile);

        if(flag.shortname != FLAGSHORT_NULL) fprintf(outfile, "'%c', ", flag.shortname);
        else                                 fputs("GONFSHORT_NULL, ", outfile);

        if(flag.is_value)                    fputs("true},\n", outfile);
        else                                 fputs("false},\n", outfile);
    }
    fputs("};\n\n", outfile);
}

static void compile_gonf_flags_by_short(struct flagspec *flags, FILE *outfile){
    flagc_t shortn;

    fputs("static const gonfc_t gonf_flags_by_short[93] = {\n", outfile);\
    for(flagc_t i = 0; i < FLAGSHORT_MAX; i++){
        shortn = flags->shortindex[i];
        if(shortn != 0){
            fprintf(outfile, "\t[%d] = %d,\n", i, shortn);
        }
    }
    fputs("};\n\n", outfile);
}

static void compile_gonf_flags_by_long(struct flagspec *flags, FILE *outfile){
    struct matchnode *longs;
    flagc_t longs_last;

    longs_last = flags->longindex->len - 1;
    longs = flags->longindex->matches;

    fputs("static struct gonf_matchlist gonf_flags_by_long[GONFLAGC] = {\n", outfile);
    for(flagc_t i = 0; i < longs_last; i++){
        fprintf(outfile, 
            "\t{%d, \"%s\", gonf_flags_by_long + %d},\n", 
            longs[i].index,
            longs[i].match,
            i + 1
        );
    }
    fprintf(outfile, 
            "\t{%d, \"%s\", gonf_flags_by_long},\n"
            "};\n\n",
            longs[longs_last].index,
            longs[longs_last].match
    );
}

static void compile_library(struct flagspec *flags, FILE *outfile){
    fwrite(gonf_head_c_dump, 1, gonf_head_c_dump_len, outfile);

    compile_gonflagc(flagspec_len(flags), outfile);
    compile_gonf_identifiers(flags, outfile);
    compile_gonf_flags(flags, outfile);
    compile_gonf_flags_by_short(flags, outfile);
    compile_gonf_flags_by_long(flags, outfile);

    fwrite(gonf_tail_c_dump, 1, gonf_tail_c_dump_len, outfile);
}

int compilegonf(struct infiles *infiles, FILE *outfile, FILE *header_outfile){
    struct flagspec *flags;
    
    /* alloc flagspec */
    flags = flagspec_new();
    if(flags == NULL)
        return COMPILEGONF_ERR_NOMEM;

    /* parse infiles */
    switch(parsegonf(infiles, flags)){
        case PARSEGONF_ERR_NOMEM:
            flagspec_free(flags);
            return COMPILEGONF_ERR_NOMEM;
        case PARSEGONF_ERR_PARSE:
            flagspec_free(flags);
            return COMPILEGONF_ERR_PARSE;
        default: break;
    }

    /* check whether flagspec is not empty */
    if(flagspec_len(flags) == 0){
        flagspec_free(flags);
        return COMPILEGONF_ERR_NOFLAGS;
    }

    /* compile lib */
    compile_library(flags, outfile);
    /* compile header */
    if(header_outfile != NULL) compile_header(flags, header_outfile);

    flagspec_free(flags);
    return COMPILEGONF_OK;
}
 