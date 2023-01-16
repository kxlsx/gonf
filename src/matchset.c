#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <matchset.h>
#include <common.h>

#define MATCHSET_SIZE_INIT 16
#define MATCHSET_MAX_FILL_COEFFICIENT 0.7

#define MATCHSET_NOITEM NULL

/* Hash the item and get its index mod STOR_SIZE */
#define MATCHSET_INDEXOF(ITEM, STOR_SIZE) \
    (hash_crc32(ITEM) % STOR_SIZE)

static uint32_t hash_crc32(char *input){
    uint32_t h;

    h = 0;
    while(*input != '\0'){
        #if defined(__x86_64__) \
        || defined(_M_X64) 
            __asm__(
                "crc32b (%%rbx), %%eax"
                : "=a" (h)
                : "a" (h), "b" (input)
            );
        #elif defined(i386) \
        || defined(__i386__) \
        || defined(__i386) \
        || defined(_M_IX86)
            __asm__(
                "crc32b (%%ebx), %%eax"
                : "=a" (h)
                : "a" (h), "b" (input)
            );
        #else
            uint32_t highorder;
            highorder = h & 0xf8000000;  
            h <<= 5;
            h ^= (highorder >> 27);
            h ^= *input;   
        #endif
        input++;
    }
    return h;
}

/* Allocate a new node and set it as parent_node's next. */
static struct matchset_node *matchset_add_node(struct matchset_node *parent_node){
    struct matchset_node *child_node;

    child_node = malloc(sizeof(struct matchset_node));
    if(child_node == NULL)
        return NULL;
    child_node->next = NULL;
    parent_node->next = child_node;
    return child_node;
}

/* Set the item of the corresponding node in the storage. */
static int matchset_fill_node(struct matchset_node *stor, gonfsize_t stor_size, char *item){
    struct matchset_node *item_node;
    gonfsize_t index;

    index = MATCHSET_INDEXOF(item, stor_size);

    item_node = stor + index;
    /* on collision: */
    if(item_node->item != MATCHSET_NOITEM){
        while(item_node->next != NULL){
            if(streq(item, item_node->item)) 
                return MATCHSET_ERR_EXISTS;
            item_node = item_node->next;
        }
        if(streq(item, item_node->item)) 
            return MATCHSET_ERR_EXISTS;

        item_node = matchset_add_node(item_node);
        if(item_node == NULL)
            return ERR_NOMEM;
    }

    item_node->item = item;
    return OK;
}

/* Free internal set storage. */
static void matchset_free_stor(struct matchset_node *stor, gonfsize_t stor_size){
    struct matchset_node *node, *tofree_node;

    /* free every list */
    for(gonfsize_t i = 0; i < stor_size; i++){
        node = stor[i].next;
        while(node != NULL){
            tofree_node = node;
            node = node->next;

            free(tofree_node);
        }
    }

    free(stor);
}

/* Reallocate the storage in the set to a new block of size new_size. */
static int matchset_realloc(struct matchset *set, gonfsize_t new_size){
    struct matchset_node *new_stor;
    struct matchset_node *tocopy_node;

    new_stor = calloc(new_size, sizeof(struct matchset_node));
    if(new_stor == NULL) 
        return ERR_NOMEM;

    for(gonfsize_t i = 0; i < set->stor_size; i++){
        tocopy_node = set->stor + i;
        if(tocopy_node->item == MATCHSET_NOITEM) 
            continue;
        do{
            if(matchset_fill_node(new_stor, new_size, tocopy_node->item) == ERR_NOMEM)
                return ERR_NOMEM;
            tocopy_node = tocopy_node->next;
        }while(tocopy_node != NULL);
    }

    /* free old storage */
    matchset_free_stor(set->stor, set->stor_size);
    /* set new storage */
    set->stor = new_stor;
    set->stor_size = new_size;
    return OK;
}

/* Resize the set storage if its significantly filled up. */
static int matchset_optimize(struct matchset *set){
    double fill_coefficient;
    gonfsize_t new_size;

    fill_coefficient = (double)(set->len) / set->stor_size;
    if(fill_coefficient < MATCHSET_MAX_FILL_COEFFICIENT)
        return OK;

    new_size = set->stor_size * 2;
    if(matchset_realloc(set, new_size) == ERR_NOMEM)
        return ERR_NOMEM;

    return OK;
}

struct matchset *matchset_new(){
    struct matchset *set;

    set = malloc(sizeof(struct matchset));
    if(set == NULL) 
        return NULL;

    set->stor = calloc(MATCHSET_SIZE_INIT, sizeof(struct matchset_node));
    if(set->stor == NULL) 
        return NULL;

    set->len = 0;
    set->stor_size = MATCHSET_SIZE_INIT;
    return set;
}


bool matchset_contains(struct matchset *set, char *item){
    struct matchset_node *item_node;
    gonfsize_t index;

    index = MATCHSET_INDEXOF(item, set->stor_size);
    item_node = set->stor + index;

    if(item_node->item == MATCHSET_NOITEM)
        return false;

    while(item_node->next != NULL){
        if(streq(item, item_node->item)) 
            return true;
        item_node = item_node->next;
    }
    if(streq(item, item_node->item)) 
        return true;

    return false;
}

int matchset_insert(struct matchset *set, char *item){
    int ret;

    ret = matchset_optimize(set);
    if(ret != OK)
        return ret;

    ret = matchset_fill_node(set->stor, set->stor_size, item);
    if(ret != OK) 
        return ret;

    set->len++;
    
    return OK;
}

void matchset_free(struct matchset *set){
    matchset_free_stor(set->stor, set->stor_size);
    free(set);
}
