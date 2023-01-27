#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <gonf.h>
#include <files.h>
#include <comp.h>
#include <common.h>

#define PRINT_ERR_NOMEM { \
    eprintf_gonf("failed to allocate memory.\n"); \
}
#define PRINT_ERR_UNIQ { \
    eprintf_gonf("output and header-file must be different.\n"); \
}
#define PRINT_ERR_CLI { \
    eprintf_gonf(); \
    gonferror_print(); \
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

static int set_infiles(struct filearr **infiles, char **args){
    gonfc_t args_count;

    args_count = gonfargc(args);

    if(args_count == 0){
        *infiles = filearr_new_stdin();
    }else{
        *infiles = filearr_new(args, args_count, "r");
    }
    if(*infiles == NULL){
        if(errno == 0){
            PRINT_ERR_NOMEM;
            return ERR_NOMEM;
        }
        return ERR_FILE;
    }
    return OK;
}

static int set_outfile(struct file *outfile){
    if(gonflag_is_present(GONFLAG_OUTPUT)){
        *outfile = file_new(
            gonflag_get_value(GONFLAG_OUTPUT), 
            "w"
        );
    }else{
        *outfile = file_new(DEFAULT_OUTFILE, "w");
    }
    if((*outfile).handle == NULL)
        return ERR_FILE;
    
    /* the stdout flag always overwrites output */
    if(gonflag_is_present(GONFLAG_STDOUT)) 
        *outfile = file_new_stdout();

    return OK;
}

static int set_header_outfile(struct file *header_outfile, char *outfile_path){
    char *header_outfile_path;

    header_outfile_path = gonflag_get_value(GONFLAG_HEADER);
    if(!gonflag_is_present(GONFLAG_STDOUT)
    && streq(header_outfile_path, outfile_path)){
        PRINT_ERR_UNIQ;
        return ERR_CLI;
    }

    *header_outfile = file_new(
        header_outfile_path,
        "w"
    );

    if(header_outfile->handle == NULL){
        return ERR_FILE;
    }
    return OK;
}

int process_args(int argc, char **argv){
    char **args_stor, **args;
    struct filearr *infiles;
    struct file outfile = {0};
    struct file header_outfile = {0};
    int ret;

    args_stor = gonfparse(argc, argv);
    args = args_stor + 1;
    if(gonferror() != GONFOK){
        if(gonferror() == GONFERR_NOMEM) {
            PRINT_ERR_NOMEM;
            return ERR_NOMEM;
        }
        PRINT_ERR_CLI;
        return ERR_CLI;
    }

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

    ret = set_infiles(&infiles, args);
    if(ret != OK){
        free(args_stor);
        return ret;
    }

    ret = set_outfile(&outfile);
    if(ret != OK){
        filearr_free(infiles);
        free(args_stor);
        return ret;
    }

    if(gonflag_is_present(GONFLAG_HEADER)){
        ret = set_header_outfile(&header_outfile, outfile.path);
        if(ret != OK){
            filearr_free(infiles);
            file_close(outfile);
            file_remove(outfile);
            free(args_stor);
            return ret;
        }
    }

    ret = compilegonf(infiles, outfile, header_outfile);

    switch(ret){
    case ERR_NOMEM:
        PRINT_ERR_NOMEM;
        __attribute__((fallthrough));
    case ERR_PARSE:
    case ERR_FILE:
    case ERR_NOFLAGS:
        file_remove(outfile);
        if(header_outfile.path != NULL)
            file_remove(header_outfile);
        __attribute__((fallthrough));
    case OK: default:
        filearr_free(infiles);
        file_close(outfile);
        if(header_outfile.path != NULL)
            file_close(header_outfile);
        free(args_stor);
        break;
    }
    return ret;
}
