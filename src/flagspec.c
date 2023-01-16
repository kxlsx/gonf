#include <stdlib.h>
#include <string.h>

#include <flagspec.h>
#include <matchset.h>
#include <common.h>

#define FLAGSPEC_SIZE_INIT 16
#define FLAGSPEC_STRPOOL_POOL_SIZE_INIT 2
#define FLAGSPEC_STRPOOL_SIZE_INIT 1024

/* Pointer to the last pool in the strpool_pool (&(strpool_pool[len - 1])) */
#define strpool_pool_last(POOL_POOL) \
    ((POOL_POOL).stor + ((POOL_POOL).len - 1))

/* Allocate a new strpool_pool and and the first pool within it. */
static struct strpool_pool strpool_pool_new(){
    struct strpool_pool strpools;

    strpools.stor = malloc(FLAGSPEC_STRPOOL_POOL_SIZE_INIT * sizeof(struct strpool));
    if(strpools.stor == NULL) 
        return strpools;
    strpools.size = FLAGSPEC_STRPOOL_POOL_SIZE_INIT;
    strpools.len = 1;
    
    strpools.stor[0].stor = malloc(FLAGSPEC_STRPOOL_SIZE_INIT);
    if(strpools.stor[0].stor == NULL) 
        return strpools;
    strpools.stor[0].size = FLAGSPEC_STRPOOL_SIZE_INIT;
    strpools.stor[0].len = 0;

    return strpools;
}

/* Free all memory associated with the passed strpool_pool.  */
static void strpool_pool_free(struct strpool_pool strpools){
    for(gonfsize_t i = 0; i < strpools.len; i++){
        free(strpools.stor[i].stor);
    }
    free(strpools.stor);
}

/* Add a new strpool to the passed strpool_pool. Return the added strpool. */
static struct strpool *strpool_pool_add_pool(struct strpool_pool *strpools, gonfsize_t min_size){
    struct strpool *last_pool, *tmp;
    gonfsize_t new_pool_size;

    strpools->len++;
    if(strpools->len == strpools->size){
        strpools->size *= 2;
        tmp = realloc(strpools->stor, strpools->size * sizeof(struct strpool));
        if(tmp == NULL) 
            return NULL;
        strpools->stor = tmp;
    }

    last_pool = strpool_pool_last(*strpools);

    new_pool_size = (last_pool - 1)->size;
    do{
        new_pool_size *= 2;
    }while(min_size >= new_pool_size);

    last_pool->stor = malloc(new_pool_size);
    if(last_pool->stor == NULL) 
        return NULL;
    last_pool->size = new_pool_size;
    last_pool->len = 0;
    return last_pool;
}

/* Allocate str in the flagsspec's strpools */
static char *flagspec_alloc_str(struct flagspec *spec, char *str, gonfsize_t strlen){
    struct strpool *last_pool;
    char *allocd_str;

    /* increment len because of null */
    strlen++;

    last_pool = strpool_pool_last(spec->strpools);    
    if(last_pool->len + strlen >= last_pool->size){
        last_pool = strpool_pool_add_pool(&(spec->strpools), strlen);
        if(last_pool == NULL)
            return NULL;
    }

    allocd_str = strcpy(last_pool->stor + last_pool->len, str);
    last_pool->len += strlen;
    return allocd_str;
}

struct flagspec *flagspec_new(void){
    struct flagspec *spec;

    spec = calloc(1, sizeof(struct flagspec));
    if(spec == NULL) return NULL;

    spec->stor = calloc(FLAGSPEC_SIZE_INIT, sizeof(struct flaginfo));
    if(spec->stor == NULL) 
        return NULL;
    spec->size = FLAGSPEC_SIZE_INIT;
    spec->last = 0;

    spec->longname_record = matchset_new();
    if(spec->longname_record == NULL) 
        return NULL;

    spec->identifier_record = matchset_new();
    if(spec->identifier_record == NULL) 
        return NULL;

    spec->strpools = strpool_pool_new();
    if(spec->strpools.stor == NULL 
    || spec->strpools.stor[0].stor == NULL)
        return NULL;

    return spec;
}

int flagspec_next(struct flagspec *spec){
    struct flaginfo *tmp;

    spec->last++;
    if(spec->last == spec->size){
        spec->size *= 2;
        tmp = realloc(spec->stor, spec->size * sizeof(struct flaginfo));
        if(tmp == NULL) return FLAGSPEC_NOMEM;

        spec->stor = tmp;
        /* set the newly allocated space to zeroes */
        memset(
            spec->stor + spec->last,
            0, 
            (spec->size - spec->last) * sizeof(struct flaginfo)
        );
    }
    return FLAGSPEC_OK;
}

/* Macro to define a function trying to set a unique text field 
 * (like identifier & longname).
 */
#define FLAGSPEC_SET_UNIQ_TEXT_FIELD_FN_DEFINE(FIELD) \
    int flagspec_set_##FIELD(struct flagspec *spec, char *FIELD, gonfsize_t len){ \
        char *str_allocd; \
        \
        /* check if unique in flagspec */ \
        if(matchset_contains(spec->FIELD##_record, FIELD)) \
            return FLAGSPEC_EXIST; \
        /* check if not already set */ \
        if(spec->stor[spec->last].FIELD != NULL) \
            return FLAGSPEC_FILLD; \
        \
        str_allocd = flagspec_alloc_str(spec, FIELD, len); \
        if(str_allocd == NULL) return FLAGSPEC_NOMEM; \
        \
        /* add to the FIELD_record and set in last */ \
        if(matchset_insert(spec->FIELD##_record, str_allocd) == MATCHSET_ERR_NOMEM) \
            return FLAGSPEC_NOMEM; \
        spec->stor[spec->last].FIELD = str_allocd; \
        \
        return FLAGSPEC_OK; \
    }
FLAGSPEC_SET_UNIQ_TEXT_FIELD_FN_DEFINE(longname)
FLAGSPEC_SET_UNIQ_TEXT_FIELD_FN_DEFINE(identifier)

int flagspec_set_shortname(struct flagspec *spec, char shortname){
    /* check if unique in flagspec */
    if(spec->shortname_record[shortname - FLAGSHORT_OFF] > 0)                   
        return FLAGSPEC_EXIST;
    /* check if not already set */
    if(spec->stor[spec->last].shortname != FLAGSHORT_NULL) 
        return FLAGSPEC_FILLD;

    /* add to the shortname_record and set in last */ 
    spec->shortname_record[shortname - FLAGSHORT_OFF] = spec->last + 1;
    spec->stor[spec->last].shortname = shortname;
    return FLAGSPEC_OK;
}

int flagspec_set_description(struct flagspec *spec, char *description, gonfsize_t len){
    char *str_allocd;

    /* check if not already set */
    if(spec->stor[spec->last].description != NULL)
        return FLAGSPEC_FILLD;

    str_allocd = flagspec_alloc_str(spec, description, len);
    if(str_allocd == NULL) return FLAGSPEC_NOMEM;

    spec->stor[spec->last].description = str_allocd;
    return FLAGSPEC_OK;
}

int flagspec_set_value(struct flagspec *spec, char *value, gonfsize_t len){
    char *str_allocd;

    str_allocd = flagspec_alloc_str(spec, value, len);
    if(str_allocd == NULL) return FLAGSPEC_NOMEM;

    spec->stor[spec->last].value = str_allocd;
    return FLAGSPEC_OK;
}

int flagspec_set_is_value(struct flagspec *spec, bool is_value){
    spec->stor[spec->last].is_value = is_value;
    return FLAGSPEC_OK;
}

void flagspec_free(struct flagspec *spec){
    free(spec->stor);

    matchset_free(spec->longname_record);
    matchset_free(spec->identifier_record);
    
    strpool_pool_free(spec->strpools);

    free(spec);
}
