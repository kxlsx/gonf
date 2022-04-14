#include <stdio.h>
#include <stdlib.h>

#include <gonf.h>
#include <comp.h>
#include <common.h>

// TODO: make code less repetetive
// TODO: figure out how to manage those big FREEING SHIT blocks

#define PRINT_ERR_NOMEM { \
    eprintf_gonf("failed to allocate memory.\n"); \
}
#define PRINT_ERR_FILE(FILENAME) { \
    eprintf_gonf(); \
    perror(FILENAME); \
}
#define PRINT_ERR_NOFLAGS { \
    eprintf_gonf("input files contain zero flags.\n"); \
}
#define PRINT_ERR_CLI { \
    eprintf_gonf(); \
    gonferror_print(); \
}

static void print_help(void){
    struct gonflag *flag;

    puts(
        NAME " v" VERSION "\n"
        AUTHORS "\n"
        DESCRIPTION "\n"
        "\n"
        "USAGE:"
    );
    printf("\t" NAME " [FLAGS]=<VALUES>... [FILES]...");
    puts(
        "\n" 
        "FLAGS:"
    );
    for(gonfc_t flagi = 0; flagi < GONFLAGC; flagi++){
        flag = gonflag_get(flagi);

        putchar('\t');
        if(flag->shortname != GONFSHORT_NONE) 
            printf("-%c ", flag->shortname);
        else
            fputs("   ", stdout);

        if(flag->longname != NULL)
            printf("--%s", flag->longname);

        if(flag->is_value)
            printf("=<%s>\n", (flag->default_value != NULL) ? flag->default_value : "VALUE");
        else
            putchar('\n');

        if(flag->description != NULL)
            printf("\t\t%s", flag->description);
        
        putchar('\n');
        putchar('\n');
    }
}

static void print_license(void){
    puts(
        NAME " v" VERSION "\n"
		AUTHORS "\n"
		"\n"
        LICENSE
    );
}

static void print_version(void){
    puts(
        NAME " v" VERSION
    );
}

static void fclose_infiles(FILE **infiles){
    while(*infiles != NULL) fclose(*(infiles++));
}

int process_args(int argc, char **argv){
    char **args_stor, **args;
    gonfc_t arglen;
    FILE **infiles;
    char *outfile_name, *header_outfile_name;
    FILE *outfile, *header_outfile;
    int compile_res;

    args_stor = gonfparse(argc, argv);
    args = args_stor + 1;
    if(gonferror() != GONFOK){
        if(gonferror() == GONFERR_NOMEM) {
            PRINT_ERR_NOMEM;
            return ERR_NOMEM;
        }
        PRINT_ERR_CLI;
        free(args_stor);
        return ERR_CLI;
    }

    if(gonflag_is_present(gonflag_get(GONFLAG_HELP))){
        print_help();
        free(args_stor);
        return OK;
    }
    if(gonflag_is_present(gonflag_get(GONFLAG_LICENSE))){
        print_license();
        free(args_stor);
        return OK;
    }
    if(gonflag_is_present(gonflag_get(GONFLAG_VERSION))){
        print_version();
        free(args_stor);
        return OK;
    }

    arglen = gonfargc(args);

    /* open input files */
    if(arglen == 0){
        infiles = malloc(2 * sizeof(FILE *));
        if(infiles == NULL){
            PRINT_ERR_NOMEM;
            free(args_stor);
            return ERR_NOMEM;
        }

        infiles[0] = stdin;
        infiles[1] = NULL;
    }else{
        infiles = malloc((arglen + 1) * sizeof(FILE *));
        if(infiles == NULL){
            PRINT_ERR_NOMEM;
            free(args_stor);
            return ERR_NOMEM;
        }

        for(gonfc_t i = 0; i < arglen; i++){
            infiles[i] = fopen(args[i], "r");
            if(infiles[i] == NULL){
                PRINT_ERR_FILE(args[i]);
                // TODO: FREEING SHIT
                fclose_infiles(infiles);
                free(infiles);
                free(args_stor);
                return ERR_FILE;
            }
        }
        infiles[arglen] = NULL;
    }

    /* open output files */
    outfile_name = gonflag_is_present(gonflag_get(GONFLAG_OUTPUT)) ?
        gonflag_get(GONFLAG_OUTPUT)->value :
        DEFAULT_OUTFILE;
    if(gonflag_is_present(gonflag_get(GONFLAG_STDOUT))){
        outfile = stdout;
        outfile_name = NULL;
    }else{
        outfile = fopen(outfile_name, "w");
        if(outfile == NULL){
            PRINT_ERR_FILE(outfile_name);
            // TODO: FREEING SHIT
            fclose_infiles(infiles);
            free(infiles);
            free(args_stor);
            return ERR_FILE;
        }
    }

    /* open header output file if needed */
    header_outfile = NULL;
    if(gonflag_is_present(gonflag_get(GONFLAG_HEADER))){
        header_outfile_name = (gonflag_get(GONFLAG_HEADER)->value == NULL) ?
            gonflag_get(GONFLAG_HEADER)->default_value :
            gonflag_get(GONFLAG_HEADER)->value;
        header_outfile = fopen(header_outfile_name, "w");
        if(header_outfile == NULL){
            PRINT_ERR_FILE(header_outfile_name);
            // TODO: FREEING SHIT
            fclose(outfile);
            if(outfile_name != NULL) remove(outfile_name);
            fclose_infiles(infiles);
            free(infiles);
            free(args_stor);
            return ERR_FILE;
        }
    }

    /* compile and check results */
    compile_res = compilegonf(infiles, outfile, header_outfile);
    if(compile_res != COMPILEGONF_OK){
        // TODO: FREEING SHIT
        if(header_outfile != NULL){
            fclose(header_outfile);
            remove(header_outfile_name);
        }
        fclose(outfile);
        if(outfile_name != NULL) remove(outfile_name);
        fclose_infiles(infiles);
        free(infiles);
        free(args_stor);
        switch(compile_res){
        case COMPILEGONF_ERR_NOMEM:
            PRINT_ERR_NOMEM;
            return ERR_NOMEM;
        case COMPILEGONF_ERR_PARSE:
            return ERR_PARSE;
        case COMPILEGONF_ERR_NOFLAGS:
            PRINT_ERR_NOFLAGS;
            return ERR_NOFLAGS;
        }
    }

    /* check for read errors */
    for(gonfc_t i = 0; i < arglen; i++){
        if(ferror(infiles[i]) != 0){
            PRINT_ERR_FILE(args[i]);
            // TODO: FREEING SHIT
            if(header_outfile != NULL){
                fclose(header_outfile);
                remove(header_outfile_name);
            }
            fclose(outfile);
            if(outfile_name != NULL) remove(outfile_name);
            fclose_infiles(infiles);
            free(infiles);
            free(args_stor);
            return ERR_FILE;
        }
    }
    /* check for write errors */
    if(ferror(outfile) != 0){
        PRINT_ERR_FILE(outfile_name);
        // TODO: FREEING SHIT
        if(header_outfile != NULL){
            fclose(header_outfile);
            remove(header_outfile_name);
        }
        fclose(outfile);
        if(outfile_name != NULL) remove(outfile_name);
        fclose_infiles(infiles);
        free(infiles);
        free(args_stor);
        return ERR_FILE;
    }
    if(header_outfile != NULL
    && ferror(header_outfile) != 0){
        PRINT_ERR_FILE(header_outfile_name);
        // TODO: FREEING SHIT
        if(header_outfile != NULL){
            fclose(header_outfile);
            remove(header_outfile_name);
        }
        fclose(outfile);
        if(outfile_name != NULL) remove(outfile_name);
        fclose_infiles(infiles);
        free(infiles);
        free(args_stor);
        return ERR_FILE;
    }

    // TODO: FREEING SHIT
    if(header_outfile != NULL) fclose(header_outfile);
    fclose(outfile);
    fclose_infiles(infiles);
    free(infiles);
    free(args_stor);
    return 0;
}
