#include <stdlib.h>
#include <string.h>

#include <flagspec.h>
#include <matchset.h>
#include <common.h>

#define FLAGSPEC_SIZE_INIT 16
#define FLAGSPEC_STRPOOL_POOL_SIZE_INIT 2
#define FLAGSPEC_STRPOOL_SIZE_INIT 1024

/* Allocate str in the flagsspec's strpool */
static char *flagspec_alloc_str(struct flagspec *spec, char *str, gonfsize_t strlen){
    struct strpool *pool, *tmp;
    gonfsize_t newsize;
    char *ret;

    /* increment len because of null */
    strlen++;

    /* fetch the last pool */
    pool = spec->strpool_pool + spec->strpool_pool_last;
    
    /* if the last pool will overflow */
    if(pool->len + strlen >= pool->size){
        /* check if there's need to realloc the strpool_pool */
        spec->strpool_pool_last++;
        newsize = pool->size;
        if(spec->strpool_pool_last == spec->strpool_pool_size){
            spec->strpool_pool_size *= 2;
            tmp = realloc(spec->strpool_pool, spec->strpool_pool_size * sizeof(struct strpool));
            if(tmp == NULL) return NULL;

            spec->strpool_pool = tmp;
        }

        /* alloc the new pool with a larger, sufficient size */
        do{
            newsize *= 2;
        }while(strlen >= newsize);
        pool = spec->strpool_pool + spec->strpool_pool_last;
        pool->stor = malloc(newsize);
        if(pool->stor == NULL) return NULL;
        pool->size = newsize;
        pool->len = 0;
    }

    /* copy the str to the pool and return its pointer */
    ret = strcpy(pool->stor + pool->len, str);
    pool->len += strlen;
    return ret;
}

struct flagspec *flagspec_new(void){
    struct flagspec *s;

    /* alloc flagspec */
    s = calloc(1, sizeof(struct flagspec));
    if(s == NULL) return NULL;

    /* alloc main flaginfo storage */
    s->stor = calloc(FLAGSPEC_SIZE_INIT, sizeof(struct flaginfo));
    if(s->stor == NULL) return NULL;
    s->size = FLAGSPEC_SIZE_INIT;
    s->last = 0;

    /* alloc the longname record */
    s->longname_record = matchset_new();
    if(s->longname_record == NULL) return NULL;
    /* alloc the identifier record */
    s->identifier_record = matchset_new();
    if(s->identifier_record == NULL) return NULL;

    /* alloc the strpool_pool */
    s->strpool_pool = malloc(FLAGSPEC_STRPOOL_POOL_SIZE_INIT * sizeof(struct strpool));
    if(s->strpool_pool == NULL) return NULL;
    s->strpool_pool_size = FLAGSPEC_STRPOOL_POOL_SIZE_INIT;
    s->strpool_pool_last = 0;
    
    /* alloc the first strpool in the pool pool */
    s->strpool_pool[0].stor = malloc(FLAGSPEC_STRPOOL_SIZE_INIT);
    if(s->strpool_pool[0].stor == NULL) return NULL;
    s->strpool_pool[0].size = FLAGSPEC_STRPOOL_SIZE_INIT;
    s->strpool_pool[0].len = 0;

    return s;
}

int flagspec_next(struct flagspec *spec){
    struct flaginfo *tmp;

    spec->last++;
    /* realloc if needed */
    if(spec->last == spec->size){
        spec->size *= 2;
        tmp = realloc(spec->stor, spec->size * sizeof(struct flaginfo));
        if(tmp == NULL) return FLAGSPEC_NOMEM;

        spec->stor = tmp;
        /* set the new allocated space to zeroes */
        memset(spec->stor + spec->last, 0, (spec->size - spec->last) * sizeof(struct flaginfo));
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
        /* add to longname_rec and set in last */ \
        if(matchset_insert(spec->FIELD##_record, str_allocd) == MATCHSET_NOMEM) \
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

    /* add to shortname_rec and set in last */ 
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
    /* free the main storage */
    free(spec->stor);
    /* free the records */
    matchset_free(spec->longname_record);
    matchset_free(spec->identifier_record);
    
    /* free every strpool and the pool pool itself */
    for(gonfsize_t i = 0; i <= spec->strpool_pool_last; i++){
        free((spec->strpool_pool + i)->stor);
    }
    free(spec->strpool_pool);

    /* free the flagspec */
    free(spec);
}
