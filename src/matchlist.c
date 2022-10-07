#include <stdlib.h>
#include <string.h>

#include <matchlist.h>

#define MATCHLIST_SIZE_INIT 16

/* Restore the pointers inside the matchlist. */
static void matchlist_restore(struct matchlist *list){
    matchc_t last;

    if(list->len > 0){
        last = list->len - 1;
        for(matchc_t i = 0; i < last; i++){
            list->matches[i].next = list->matches + i + 1;
        }
        list->matches[last].next = list->matches;
    }
}

struct matchlist *matchlist_new(){
    struct matchlist *m;

    m = malloc(sizeof(struct matchlist));
    if(m == NULL) return NULL;
    
    m->len = 0;
    m->size = MATCHLIST_SIZE_INIT;
    m->matches = malloc(MATCHLIST_SIZE_INIT * sizeof(struct matchnode));
    if(m->matches == NULL) return NULL;

    return m;
}

void matchlist_free(struct matchlist *list){
    free(list->matches);
    free(list);
}

int matchlist_append(struct matchlist *list, char *match, matchc_t index){
    struct matchnode *tmp;

    if(list->len == list->size){
        list->size *= 2;
        tmp = realloc(list->matches, list->size * sizeof(struct matchnode));
        if(tmp == NULL) return MATCHLIST_NOMEM;

        list->matches = tmp;
        matchlist_restore(list);
    }

    tmp = list->matches + list->len;
    tmp->index = index;
    tmp->match = match;
    if(list->len != 0) (tmp - 1)->next = tmp;
    tmp->next = list->matches;

    list->len += 1;
    return MATCHLIST_OK;
}

int match_find(char *needle, struct matchlist *list){
    struct matchnode *curr, *prev;
    matchc_t count, prev_count;
    matchc_t needle_len;
    matchc_t stepc;

    count = list->len;
    needle_len = strlen(needle);

    curr = list->matches;
    prev = &(list->matches[count - 1]);

    for(stepc = 0; count != 0 && stepc <= needle_len; stepc++){
        prev_count = count;
        for(matchc_t i = 0; i < prev_count; i++){
            if(curr->match[stepc] != needle[stepc]){
                count--;
                prev->next = curr->next;
            }else{
                prev = curr;
            }
            curr = curr->next;
        }
    }

    matchlist_restore(list);
    return (count != 0) ? (int)curr->index : -1;
}
