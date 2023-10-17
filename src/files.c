#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include <files.h>
#include <common.h>

#define PRINT_ERR_FILE(FILENAME) { \
    eprintf_gonf(); \
    perror(FILENAME); \
}

static bool isdir(char *path){
    struct stat path_stat = {0};
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

struct file file_new(char *path, const char *restrict mode){
    struct file f = {0};

    if(isdir(path)){
        errno = EISDIR;
        PRINT_ERR_FILE(path);
        return f;
    }

    f.path = path;
    f.handle = fopen(path, mode);

    if(f.handle == NULL)
        PRINT_ERR_FILE(path);
    return f;
}

struct file file_new_stdin(void){
    return (struct file){stdin, "stdin"};
}

struct file file_new_stdout(void){
    return (struct file){stdout, "stdout"};
}

int file_error_check(struct file file){
    if(ferror(file.handle) != 0){
        PRINT_ERR_FILE(file.path);
        return ERR_FILE;
    }
    return OK;
}

struct filearr *filearr_new(char **paths, gonfsize_t count, const char *restrict modes){
    struct filearr *filearr;

    filearr = malloc(sizeof(struct filearr));
    if(filearr == NULL)
        return NULL;

    filearr->files = malloc(count * sizeof(struct file));
    if(filearr->files == NULL)
        return NULL;
    
    filearr->len = count;
    for(gonfsize_t i = 0; i < count; i++){
        filearr->files[i] = file_new(paths[i], modes);

        if(filearr->files[i].handle == NULL){
            filearr->len = i;
            filearr_free(filearr);
            return NULL;
        }
    }

    return filearr;
}

struct filearr *filearr_new_stdin(void){
    struct filearr *filearr;

    filearr = malloc(sizeof(struct filearr));
    if(filearr == NULL)
        return NULL;

    filearr->files = malloc(sizeof(struct file));
    if(filearr->files == NULL)
        return NULL;

    filearr->len = 1;
    filearr->files[0] = file_new_stdin();
    
    return filearr;
}

int filearr_check_errors(struct filearr *filearr){
    bool is_err;
    
    is_err = false;
    for(gonfsize_t i = 0; i < filearr->len; i++){
        if(file_error_check(filearr->files[i]) == ERR_FILE)
            is_err = true;
    }
    return is_err ? ERR_FILE : OK;
}

void filearr_free(struct filearr *filearr){
    for(gonfsize_t i = 0; i < filearr->len; i++){
        file_close(filearr->files[i]);
    }
    free(filearr->files);
    free(filearr);
}
