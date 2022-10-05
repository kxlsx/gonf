#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h> //TODO: check if works on windows

#include <common.h>
#include <gonf.h>
#include <infiles.h>
#include <comp.h>

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
#define PRINT_ERR_UNIQ { \
    eprintf_gonf("output and header-file must be different.\n"); \
}
#define PRINT_ERR_CLI { \
    eprintf_gonf(); \
    gonferror_print(); \
}

void infiles_free(struct infiles *infiles){
    for(gonsize_t i = 0; i < infiles->len; i++){
        fclose(infiles->farr[i]);
    }
    free(infiles->farr);
    free(infiles->parr);
    free(infiles);
}

/* Check if the provided path points to a directory */
static bool isdir(char *path){
    struct stat path_stat = {0};
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

/* Create a new infiles struct, opening every file in file_path.
 * On error, writes to stderr and returns NULL.
 */
static struct infiles *infiles_new(char **file_paths, gonsize_t count){
    struct infiles *infiles;

    infiles = malloc(sizeof(struct infiles));
    if(infiles == NULL){
        PRINT_ERR_NOMEM;
        return NULL;
    }

    infiles->farr = malloc(count * sizeof(FILE *));
    infiles->parr = malloc(count * sizeof(char *));
    if(infiles->farr == NULL || infiles->parr == NULL){
        PRINT_ERR_NOMEM;
        free(infiles);
        return NULL;
    }
    infiles->len = count;

    /* copy file paths */
    memcpy(infiles->parr, file_paths, count * sizeof(char *));
    /* open files */
    for(gonsize_t i = 0; i < count; i++){
        infiles->farr[i] = fopen(infiles->parr[i], "r");
        /* check if is a directory */
        if(isdir(infiles->parr[i])){
            errno = EISDIR;
            fclose(infiles->farr[i]);
            infiles->farr[i] = NULL;
        }
        /* check if file is opened */
        if(infiles->farr[i] == NULL){
            PRINT_ERR_FILE(infiles->parr[i]);

            /* close previously opened files */            
            while(i != 0){
                i--;
                fclose(infiles->farr[i]);
            }
            /* free storage */
            free(infiles->farr);
            free(infiles->parr);
            free(infiles);
            return NULL;
        }
    }

    return infiles;
}

/* Create a new infiles struct containing just stdin
 * On error, writes to stderr and returns NULL.
 */
static struct infiles *infiles_new_stdin(){
    struct infiles *infiles;

    infiles = malloc(sizeof(struct infiles));
    if(infiles == NULL){
        PRINT_ERR_NOMEM;
        return NULL;
    }

    infiles->farr = malloc(sizeof(FILE *));
    infiles->parr = malloc(sizeof(char *));
    if(infiles->farr == NULL || infiles->parr == NULL){
        PRINT_ERR_NOMEM;
        free(infiles);
        return NULL;
    }
    infiles->len = 1;

    infiles->farr[0] = stdin;
    infiles->parr[0] = "stdin";
    
    return infiles;
}

static void print_help(void){
    struct gonflag *flag;

    fputs(
        NAME " v" VERSION "\n"
        AUTHORS "\n"
        DESCRIPTION "\n"
        "\n"
        "USAGE:"
        "\n"
        "\t" NAME " [FLAGS]=<VALUES>... [FILES]..."
        "\n" 
        "FLAGS:",
        stdout
    );
    for(gonfc_t flagi = 0; flagi < GONFLAGC; flagi++){
        flag = gonflag_get(flagi);

        putchar('\n');
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

int process_args(int argc, char **argv){
    char **args_stor, **args;
    gonfc_t argslen;
    struct infiles *infiles;
    char *outfile_name, *header_outfile_name;
    int compile_res;

    /* read flags and arguments from argv */
    args_stor = gonfparse(argc, argv);
    args = args_stor + 1;
    argslen = gonfargc(args);
    if(gonferror() != GONFOK){
        if(gonferror() == GONFERR_NOMEM) {
            PRINT_ERR_NOMEM;
            return ERR_NOMEM;
        }
        PRINT_ERR_CLI;
        free(args_stor);
        return ERR_CLI;
    }

    /* print information if requested */
    if(gonflag_is_present(GONFLAG_HELP)){
        print_help();
        free(args_stor);
        return OK;
    }
    if(gonflag_is_present(GONFLAG_LICENSE)){
        print_license();
        free(args_stor);
        return OK;
    }
    if(gonflag_is_present(GONFLAG_VERSION)){
        print_version();
        free(args_stor);
        return OK;
    }

    /* set output file name */
    outfile_name = gonflag_is_present(GONFLAG_OUTPUT) ?
        gonflag_get_field(GONFLAG_OUTPUT, value) :
        DEFAULT_OUTFILE;
    if(gonflag_is_present(GONFLAG_STDOUT))
        outfile_name = NULL;

    /* set header output file name if needed */
    if(gonflag_is_present(GONFLAG_HEADER)){
        header_outfile_name = gonflag_get_field(GONFLAG_HEADER, value);
        /* check whether outfile_name and header_outfile_name are different */
        if(outfile_name != NULL
        && strcmp(header_outfile_name, outfile_name) == 0){
            PRINT_ERR_UNIQ;

            free(args_stor);
            return ERR_CLI;
        }
    }else{
        header_outfile_name = NULL;
    }

    /* open input files */
    infiles = (argslen == 0) ? infiles_new_stdin() : infiles_new(args, argslen);
    if(infiles == NULL){
        free(args_stor);
        return ERR_FILE;
    }
    
    /* compile and check results */
    compile_res = compilegonf(infiles, outfile_name, header_outfile_name);
    if(compile_res != COMPILEGONF_OK){
        infiles_free(infiles);
        free(args_stor);
        switch(compile_res){
        case COMPILEGONF_ERR_NOMEM:
            PRINT_ERR_NOMEM;
            return ERR_NOMEM;
        case COMPILEGONF_ERR_FILE:
            return ERR_FILE;
        case COMPILEGONF_ERR_PARSE:
            return ERR_PARSE;
        case COMPILEGONF_ERR_NOFLAGS:
            PRINT_ERR_NOFLAGS;
            return ERR_NOFLAGS;
        }
    }

    /* check for read errors */
    for(gonfc_t i = 0; i < argslen; i++){
        if(ferror(infiles_get_file(infiles, i)) != 0){
            PRINT_ERR_FILE(args[i]);

            infiles_free(infiles);
            free(args_stor);
            return ERR_FILE;
        }
    }

    infiles_free(infiles);
    free(args_stor);
    return 0;
}
