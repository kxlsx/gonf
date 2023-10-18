#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define $POK             0
#define $PERR_NOMEM      1
#define $PERR_NOFLAG     2
#define $PERR_UNKNFLAG   3
#define $PERR_NOTVALFLAG 4
#define $PERR_NOVAL      5

#define $PSHORT_NULL '\0'

typedef int $pc_t;
typedef int $psize_t;

struct $plag{
    char *default_value;
    char *value;
    const char *const description;
    const char *const longname;
    $pc_t count;
    const char shortname;
    const bool is_value;
};

struct $p_matchlist{
    const $pc_t index;
    const char *const match;
    struct $p_matchlist *next;
};$s
#define eprintf(FMT, ...) fprintf(stderr, FMT, ## __VA_ARGS__)

static struct {
    int errid;
    const char *value;
    $psize_t value_len;
} $p_err;

/* Initial size of the dynamic array returned from
 * $pparse. Change it if you expect a lot of non-flag args.
 */
#define $P_ARGS_SIZE_INIT 8

static struct {
    char **stor;
    $pc_t size;
    $pc_t len;
} $p_args;


static void $p_err_set(int errid, const char *v, $psize_t vlen){
    $p_err.value = v;
    $p_err.value_len = vlen;
    $p_err.errid = errid;
}

static int $p_args_init(){
    $p_args.len = 0;
    $p_args.size = $P_ARGS_SIZE_INIT;
    $p_args.stor = malloc($P_ARGS_SIZE_INIT * sizeof(char **) + 1);
    if($p_args.stor == NULL){
        $p_err_set($PERR_NOMEM, NULL, 0);
        return $PERR_NOMEM;
    }
    return $POK;
}

static int $p_args_push(char *arg) {
    if($p_args.len == $p_args.size){
        $p_args.size = $p_args.size * 2;
        $p_args.stor = realloc($p_args.stor, $p_args.size);
        if($p_args.stor == NULL){
            $p_err_set($PERR_NOMEM, NULL, 0);
            return $PERR_NOMEM;
        }
    }
    $p_args.stor[$p_args.len] = arg;
    $p_args.len++;
    return $POK;
}

static void $p_args_free(void){
    free($p_args.stor);
}

$pc_t $pargc(void){
    return $p_args.len;
}

struct $plag *$plag_get($pc_t flag_index){
    return (flag_index >= 0 && flag_index < $PLAGC) ?
        $p_flags + flag_index :
        NULL;
}

struct $plag *$plag_get_by_short(char shortname){
    $pc_t index;
    
    index = $p_flags_by_short[shortname - 33] - 1;
    return (index >= 0) ? 
        $p_flags + index :
        NULL;
}

struct $plag *$plag_get_by_long(const char *longname){
    struct $p_matchlist *curr, *prev;
    $pc_t count, prev_count;
    $psize_t name_len;
    $psize_t stepc;

    count = $PLAGC;
    name_len = strlen(longname);

    curr = $p_flags_by_long;
    prev = $p_flags_by_long + $PLAGC - 1;

    for(stepc = 0; count != 0 && stepc <= name_len; stepc++){
        prev_count = count;
        for($pc_t i = 0; i < prev_count; i++){
            if(curr->match[stepc] != longname[stepc]){
                count--;
                prev->next = curr->next;
            }else{
                prev = curr;
            }
            curr = curr->next;
        }
    }

    for($pc_t i = 0; i < $PLAGC - 1; i++){
        $p_flags_by_long[i].next = $p_flags_by_long + i + 1;
    }
    $p_flags_by_long[$PLAGC - 1].next = $p_flags_by_long;
    return (count != 0) ? $p_flags + curr->index : NULL;
}

static struct $plag *$p_parse_short(char *shortflags){
    struct $plag *flag;

    if(*shortflags == '\0' || *shortflags == '='){
        $p_err_set($PERR_NOFLAG, "-", 1);
        return NULL;
    }
    do{
        flag = $plag_get_by_short(*shortflags);
        if(flag == NULL){
            $p_err_set($PERR_UNKNFLAG, shortflags, 1);
            return NULL;
        }
        flag->count++;

        shortflags++;
        if(*shortflags == '='){
            if(flag->is_value){
                flag->value = shortflags + 1;
                return flag;
            }else{
                $p_err_set($PERR_NOTVALFLAG, shortflags - 1, 1);
                return NULL;
            }
        }else if(flag->is_value
        && *shortflags != '\0'){
            flag->value = shortflags;
            return flag;
        }
    }while(*shortflags != '\0');
    return flag;
}

static struct $plag *$p_parse_long(char *longflag){
    struct $plag *flag;
    char *longflag_buf;
    $psize_t i;

    if(*longflag == '='){
        $p_err_set($PERR_NOFLAG, "--", 2);
        return NULL;
    }

    longflag_buf = malloc(strlen(longflag) + 1);
    if(longflag_buf == NULL){
        $p_err_set($PERR_NOMEM, NULL, 0);
        return NULL;
    }

    flag = NULL;
    i = 0;
    do{
        if(longflag[i] == '\0'){
            $p_err_set($PERR_UNKNFLAG, longflag, strlen(longflag));
            free(longflag_buf);
            return NULL;
        }

        longflag_buf[i] = longflag[i];
        i++;
        longflag_buf[i] = '\0';

        flag = $plag_get_by_long(longflag_buf);
    }while(flag == NULL);
    flag->count++;

    if(longflag[i] != '\0'){
        if(flag->is_value){
            if(longflag[i] == '=') i++;

            flag->value =longflag + i;
        }else{
            $p_err_set($PERR_NOTVALFLAG, flag->longname, strlen(flag->longname));
            free(longflag_buf);
            return NULL;
        }
    }

    free(longflag_buf);
    return flag;
}

char **$pparse($pc_t argc, char **argv){
    enum{
        DEFAULT,
        VALUE_OPTIONAL,
        VALUE_REQUIRED,
        ARGS_ONLY,
    }parse_state;
    struct $plag *flag;

    $p_err_set($POK, NULL, 0);

    if($p_args_init() != $POK) 
        return NULL;

    flag = NULL;
    parse_state = DEFAULT;
    for($pc_t i = 0; i < argc; i++){
        if(parse_state != ARGS_ONLY && *(argv[i]) == '-'){
            if(parse_state == VALUE_REQUIRED) break;
            if(parse_state == VALUE_OPTIONAL) flag->value = flag->default_value;

            argv[i]++;
            if(*(argv[i]) == '-') {
                argv[i]++;

                if(*argv[i] == '\0') {
                    parse_state = ARGS_ONLY;
                    continue;
                }

                flag = $p_parse_long(argv[i]);
            }else {
                flag = $p_parse_short(argv[i]);
            }

            if(flag == NULL){
                $p_args_free();
                return NULL;
            }

            if(flag->is_value && flag->value == NULL){
                if(flag->default_value == NULL)
                    parse_state = VALUE_REQUIRED;
                else 
                    parse_state = VALUE_OPTIONAL;
            } else {
                parse_state = DEFAULT;
            }
        }else{
            switch (parse_state){
            case VALUE_REQUIRED: case VALUE_OPTIONAL:
                flag->value = argv[i];
                parse_state = DEFAULT;
                break;
            default:
                if($p_args_push(argv[i]) != $POK)
                    return NULL;
                break;
            }
        }
    }
    switch(parse_state){
    case VALUE_REQUIRED:
        if(flag->longname != NULL){
            $p_err_set($PERR_NOVAL, flag->longname, strlen(flag->longname));
        }else{
            $p_err_set($PERR_NOVAL, &(flag->shortname), 1);
        }
        $p_args_free();
        return NULL;
    case VALUE_OPTIONAL:
        flag->value = flag->default_value;
        break;
    default: break;
    }

    return $p_args.stor;
}

int $perror(void){
    return $p_err.errid;
}

const char *$perror_value(void){
    return $p_err.value;
}

int $perror_print(void){
    switch ($p_err.errid){
    case $PERR_NOMEM:
        eprintf("failed to allocate memory.\n");
        break;
    case $PERR_NOFLAG:
        eprintf("no flag name provided after '%.*s'.\n", $p_err.value_len, $p_err.value);
        break;
    case $PERR_UNKNFLAG:
        eprintf("unrecognized flag '%.*s'.\n", $p_err.value_len, $p_err.value);
        break;
    case $PERR_NOTVALFLAG:
        eprintf("flag '%.*s' does not take a value.\n", $p_err.value_len, $p_err.value);
        break;
    case $PERR_NOVAL:
        eprintf("no value provided for '%.*s'.\n", $p_err.value_len, $p_err.value);
        break;
    default:
        break;
    }
    return $p_err.errid;
}
