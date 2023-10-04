#define eprintf(FMT, ...) fprintf(stderr, FMT, ## __VA_ARGS__)

static struct {
    int errid;
    const char *value;
    gonfsize_t value_len;
} gonf_err;

/* Initial size of the dynamic array returned from
 * gonfparse. Change it if you expect a lot of non-flag args.
 */
#define GONF_ARGS_SIZE_INIT 8

static struct {
    char **stor;
    gonfc_t size;
    gonfc_t len;
} gonf_args;


static void gonf_err_set(int errid, const char *v, gonfsize_t vlen){
    gonf_err.value = v;
    gonf_err.value_len = vlen;
    gonf_err.errid = errid;
}

static int gonf_args_init(){
    gonf_args.len = 0;
    gonf_args.size = GONF_ARGS_SIZE_INIT;
    gonf_args.stor = malloc(GONF_ARGS_SIZE_INIT * sizeof(char **) + 1);
    if(gonf_args.stor == NULL){
        gonf_err_set(GONFERR_NOMEM, NULL, 0);
        return GONFERR_NOMEM;
    }
    return GONFOK;
}

static int gonf_args_push(char *arg) {
    if(gonf_args.len == gonf_args.size){
        gonf_args.size = gonf_args.size * 2;
        gonf_args.stor = realloc(gonf_args.stor, gonf_args.size);
        if(gonf_args.stor == NULL){
            gonf_err_set(GONFERR_NOMEM, NULL, 0);
            return GONFERR_NOMEM;
        }
    }
    gonf_args.stor[gonf_args.len] = arg;
    gonf_args.len++;
    return GONFOK;
}

static void gonf_args_free(void){
    free(gonf_args.stor);
}

gonfc_t gonfargc(void){
    return gonf_args.len;
}

struct gonflag *gonflag_get(gonfc_t flag_index){
    return (flag_index >= 0 && flag_index < GONFLAGC) ?
        gonf_flags + flag_index :
        NULL;
}

struct gonflag *gonflag_get_by_short(char shortname){
    gonfc_t index;
    
    index = gonf_flags_by_short[shortname - 33] - 1;
    return (index >= 0) ? 
        gonf_flags + index :
        NULL;
}

struct gonflag *gonflag_get_by_long(const char *longname){
    struct gonf_matchlist *curr, *prev;
    gonfc_t count, prev_count;
    gonfsize_t name_len;
    gonfsize_t stepc;

    count = GONFLAGC;
    name_len = strlen(longname);

    curr = gonf_flags_by_long;
    prev = gonf_flags_by_long + GONFLAGC - 1;

    for(stepc = 0; count != 0 && stepc <= name_len; stepc++){
        prev_count = count;
        for(gonfc_t i = 0; i < prev_count; i++){
            if(curr->match[stepc] != longname[stepc]){
                count--;
                prev->next = curr->next;
            }else{
                prev = curr;
            }
            curr = curr->next;
        }
    }

    for(gonfc_t i = 0; i < GONFLAGC - 1; i++){
        gonf_flags_by_long[i].next = gonf_flags_by_long + i + 1;
    }
    gonf_flags_by_long[GONFLAGC - 1].next = gonf_flags_by_long;
    return (count != 0) ? gonf_flags + curr->index : NULL;
}

static struct gonflag *gonf_parse_short(char *shortflags){
    struct gonflag *flag;

    if(*shortflags == '\0' || *shortflags == '='){
        gonf_err_set(GONFERR_NOFLAG, "-", 1);
        return NULL;
    }
    do{
        flag = gonflag_get_by_short(*shortflags);
        if(flag == NULL){
            gonf_err_set(GONFERR_UNKNFLAG, shortflags, 1);
            return NULL;
        }
        flag->count++;

        shortflags++;
        if(*shortflags == '='){
            if(flag->is_value){
                flag->value = shortflags + 1;
                return flag;
            }else{
                gonf_err_set(GONFERR_NOTVALFLAG, shortflags - 1, 1);
                return NULL;
            }
        }else if(flag->is_value
        && *shortflags != '\0'
        && flag->default_value == NULL){
            gonf_err_set(GONFERR_NOVAL, shortflags - 1, 1);
            return NULL;
        }
    }while(*shortflags != '\0');
    return flag;
}

static struct gonflag *gonf_parse_long(char *longflag){
    struct gonflag *flag;
    char *value;

    if(*longflag == '='){
        gonf_err_set(GONFERR_NOFLAG, "--", 2);
        return NULL;
    }

    value = strchr(longflag, '=');
    if(value != NULL){
        *value = '\0';
        value++;
    }

    flag = gonflag_get_by_long(longflag);
    if(flag == NULL){
        gonf_err_set(GONFERR_UNKNFLAG, longflag, strlen(longflag));
        return NULL;
    }
    flag->count++;

    if(value != NULL){
        if(!flag->is_value){
            gonf_err_set(GONFERR_NOTVALFLAG, longflag, strlen(longflag));
            return NULL;
        }
        flag->value = value;
    }

    return flag;
}

char **gonfparse(gonfc_t argc, char **argv){
    enum{
        DEFAULT,
        VALUE_OPTIONAL,
        VALUE_REQUIRED,
        ARGS_ONLY,
    }parse_state;
    struct gonflag *flag;

    gonf_err_set(GONFOK, NULL, 0);

    if(gonf_args_init() != GONFOK) 
        return NULL;

    flag = NULL;
    parse_state = DEFAULT;
    for(gonfc_t i = 0; i < argc; i++){
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

                flag = gonf_parse_long(argv[i]);
            }else {
                flag = gonf_parse_short(argv[i]);
            }

            if(flag == NULL){
                gonf_args_free();
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
                if(gonf_args_push(argv[i]) != GONFOK)
                    return NULL;
                break;
            }
        }
    }
    switch(parse_state){
    case VALUE_REQUIRED:
        if(flag->longname != NULL){
            gonf_err_set(GONFERR_NOVAL, flag->longname, strlen(flag->longname));
        }else{
            gonf_err_set(GONFERR_NOVAL, &(flag->shortname), 1);
        }
        gonf_args_free();
        return NULL;
    case VALUE_OPTIONAL:
        flag->value = flag->default_value;
        break;
    default: break;
    }

    return gonf_args.stor;
}

int gonferror(void){
    return gonf_err.errid;
}

const char *gonferror_value(void){
    return gonf_err.value;
}

int gonferror_print(void){
    switch (gonf_err.errid){
    case GONFERR_NOMEM:
        eprintf("failed to allocate memory.\n");
        break;
    case GONFERR_NOFLAG:
        eprintf("no flag name provided after '%.*s'.\n", gonf_err.value_len, gonf_err.value);
        break;
    case GONFERR_UNKNFLAG:
        eprintf("unrecognized flag '%.*s'.\n", gonf_err.value_len, gonf_err.value);
        break;
    case GONFERR_NOTVALFLAG:
        eprintf("flag '%.*s' does not take a value.\n", gonf_err.value_len, gonf_err.value);
        break;
    case GONFERR_NOVAL:
        eprintf("no value provided for '%.*s'.\n", gonf_err.value_len, gonf_err.value);
        break;
    default:
        break;
    }
    return gonf_err.errid;
}
