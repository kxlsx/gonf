#ifndef _FILES_H
/* ================= */
#define _FILES_H 1

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _COMMON_H
#include <common.h>
#endif

/* Struct representing an opened file stored with its path. */
struct file{
    FILE *handle;
    char *path;
};

/* Struct representing an array of files. */
struct filearr{
    struct file *files;
    gonfsize_t len;
};

/* Close the FILE */
#define file_close(FILE) \
    fclose(FILE.handle)
/* Remove the FILE */
#define file_remove(FILE){ \
    remove(FILE.path); \
}

/* Open the file in path and return it as a struct file. */
struct file file_new(char *path, const char *restrict mode);
/* Return a struct file containing stdin's handle. */
struct file file_new_stdin(void);
/* Return a struct file containing stdouts's handle. */
struct file file_new_stdout(void);
/* Check for errors in the passed file.
 * If an error is found, a message is printed
 * to stderr and the function returns ERR_FILE.
 * 
 * RETURNS:
 *  OK
 *  or
 *  ERR_FILE
 */
int file_error_check(struct file file);

/* Allocate a new filearr of size count 
 * containing files in the passed paths.
 * The filearr does not take ownership or copy
 * the passsed paths.
 */
struct filearr *filearr_new(char **paths, gonfsize_t count, const char *restrict modes);
/* Allocate a new filearr containing only stdin. */
struct filearr *filearr_new_stdin(void);
/* Check for errors in the files in the passed filearr. 
 * If an error is found, a message is printed
 * to stderr and the function returns ERR_FILE.
 * 
 * RETURNS:
 *  OK
 *  or
 *  ERR_FILE
 */
int filearr_check_errors(struct filearr *filearr);
/* Close every file in the filearr 
 * and free all memory associated with it.
 */
void filearr_free(struct filearr *filearr);

/* ================= */
#endif