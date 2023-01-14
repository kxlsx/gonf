#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <matchset.h>

#define MATCHSET_SIZE_INIT 16
#define MATCHSET_MAX_FILL_COEFFICIENT 0.7

static uint32_t hash_crc32(char *input){
    uint32_t h;

    h = 0;
    while(*input != '\0'){
        /* I'm not explaining myself. */
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

/* Set the item of the corresponding node in the storage. */
static int matchset_set_node(struct matchset_node *stor, gonfsize_t stor_size, char *item){
    struct matchset_node *node;
    gonfsize_t index;

    /* hash the item and calculate its index in the set*/
    index = hash_crc32(item) % stor_size;

    node = stor + index;
    /* on collision:
     * append a new node at the end of the list
     */
    if(node->item != NULL){
        while(node->next != NULL){
            if(strcmp(item, node->item) == 0) return MATCHSET_EXISTS;
            node = node->next;
        }
        if(strcmp(item, node->item) == 0) return MATCHSET_EXISTS;

        /* alloc new node */
        node->next = malloc(sizeof(struct matchset_node));
        if(node->next == NULL) return MATCHSET_NOMEM;

        node = node->next;
        node->next = NULL;
    }
    node->item = item;

    return MATCHSET_OK;
}

/* Free internal set storage. */
static void matchset_free_stor(struct matchset_node *stor, gonfsize_t stor_size){
    struct matchset_node *node, *tofree_node;

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

/* Reallocate the storage in self to a new block of size new_size. */
static int matchset_realloc(struct matchset *self, gonfsize_t new_size){
    struct matchset_node *new_stor;
    struct matchset_node *node;

    /* alloc new storage */
    new_stor = calloc(new_size, sizeof(struct matchset_node));
    if(new_stor == NULL) return MATCHSET_NOMEM;

    /* copy every item from self->stor to new_stor */
    for(gonfsize_t i = 0; i < self->stor_size; i++){
        node = self->stor + i;
        if(node->item == NULL) continue;
        do{
            if(matchset_set_node(new_stor, new_size, node->item) == MATCHSET_NOMEM)
                return MATCHSET_NOMEM;
            node = node->next;
        }while(node != NULL);
    }

    /* free old storage */
    matchset_free_stor(self->stor, self->stor_size);
    /* set new storage */
    self->stor = new_stor;
    self->stor_size = new_size;
    return MATCHSET_OK;
}

/* Resize the set storage if its significantly filled up. */
static int matchset_optimize(struct matchset *self){
    double fill_coefficient;
    gonfsize_t new_size;

    /* check how filled up is the set */
    fill_coefficient = (double)(self->len) / self->stor_size;
    if(fill_coefficient < MATCHSET_MAX_FILL_COEFFICIENT) return MATCHSET_OK;

    /* resize the set */
    new_size = self->stor_size * 2;
    if(matchset_realloc(self, new_size) == MATCHSET_NOMEM) return MATCHSET_NOMEM;

    return MATCHSET_OK;
}

struct matchset *matchset_new(){
    struct matchset *m;

    /* alloc set */
    m = malloc(sizeof(struct matchset));
    if(m == NULL) return NULL;

    /* alloc set storage */
    m->stor = calloc(MATCHSET_SIZE_INIT, sizeof(struct matchset_node));
    if(m->stor == NULL) return NULL;

    m->len = 0;
    m->stor_size = MATCHSET_SIZE_INIT;
    return m;
}


bool matchset_contains(struct matchset *self, char *item){
    struct matchset_node *node;
    gonfsize_t index;

    /* hash the item and calculate its index in the set*/
    index = hash_crc32(item) % self->stor_size;

    node = self->stor + index;

    /* no item at specified index */
    if(node->item == NULL) return false;

    /* check the list to find the item*/
    while(node->next != NULL){
        if(strcmp(item, node->item) == 0) return true;
        node = node->next;
    }
    if(strcmp(item, node->item) == 0) return true;

    return false;
}

int matchset_insert(struct matchset *self, char *item){
    int ret;

    /* optimize the set (realloc if sufficiently filled up) */
    if(matchset_optimize(self) == MATCHSET_NOMEM)
        return MATCHSET_NOMEM;

    /* insert the item into the set */
    ret = matchset_set_node(self->stor, self->stor_size, item);
    if(ret != MATCHSET_OK) return ret;

    self->len++;
    return MATCHSET_OK;
}


void matchset_free(struct matchset *self){
    matchset_free_stor(self->stor, self->stor_size);
    free(self);
}
