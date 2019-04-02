/*
  NAME: Saibalaji Atmakuri
  EMAIL: saiatmakuri@yahoo.com
  ID: 004963466
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include "SortedList.h"

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
	
	if (list == NULL || element == NULL || list->key != NULL) {
		return;
	}
	
	SortedList_t* it = list->next;
	
	while (it != list) {
		if (strcmp(element->key, it->key) < 0) {
			break;
		}
		it = it->next;
	}

	if (opt_yield & INSERT_YIELD) {
		sched_yield();
	}

	element->next = it->next;
	it->next = element;
	element->next->prev = element;
	element->prev = it;

}

/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *	The specified element will be removed from whatever
 *	list it is currently in.
 *
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */
int SortedList_delete(SortedListElement_t *element) {
	if (element == NULL || element->key == NULL || element->next->prev != element || element->prev->next != element) {
		return 1;
	}

	if (opt_yield & DELETE_YIELD) {
		sched_yield();
	}

	element->prev->next = element->next;
	element->next->prev = element->prev;
	free(element);
	return 0;	
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *	The specified list will be searched for an
 *	element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
	if (list == NULL || list->key != NULL || key == NULL) {
		return NULL;
	}

	SortedList_t* it = list->next;

	while (it != list) {
		if (strcmp(it->key, key) == 0) {
			return it;
		}
		if (opt_yield & LOOKUP_YIELD) {
			sched_yield();
		}
		it = it->next;
	}
	return NULL;
}

/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list) {
	if (list == NULL || list->key != NULL) {
		return -1;
	}

	int count = 0;
	SortedList_t* it = list->next;
	while (it != list) {
		if (it->next->prev != it || it->prev->next != it) {
			return -1;
		}
		count++;
		if (opt_yield & LOOKUP_YIELD) {
			sched_yield();
		}
		it = it->next;
	}
	return count;
}
