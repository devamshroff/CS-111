

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){

    if(list == NULL || element == NULL || list->key != NULL)
        return;
   
    SortedList_t* iter = list->next;
    while(iter != list){
        if(strcmp(element->key, iter->key) < 0){
            break;
        }
        iter = iter->next;
    }
   
    if (opt_yield & INSERT_YIELD)
        sched_yield();

    element->next = iter->next;
    iter->next = element;
    element->next->prev = element;
    element->prev = iter;

}

int SortedList_delete( SortedListElement_t *element){
    if(element == NULL || element->key == NULL || element->next->prev != element || element->prev->next != element){
        return 1;
    }

    if (opt_yield & DELETE_YIELD){
        sched_yield();
    }
    
    element->prev->next = element->next;
    element->next->prev = element->prev;
    free(element);
    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
    if(list == NULL || list->key != NULL || key == NULL)
        return NULL;

    SortedList_t* iter = list->next;
    
    while(iter != list){
        if(strcmp(iter->key, key) == 0)
            return iter;
        if (opt_yield & LOOKUP_YIELD)
            sched_yield();
        iter = iter->next;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list){
    if(list == NULL || list->key != NULL)
        return -1;

    int count = 0;
    SortedList_t* iter = list->next;
    while(iter != list){
        if (iter->prev->next != iter || iter->next->prev != iter)
            return -1;
        count++;
        if (opt_yield & LOOKUP_YIELD)
            sched_yield();
        iter = iter->next;
    }
    return count;
}