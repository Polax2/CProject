//
// Created by polan on 20/12/2024.
//all code taken from previous labs
//
#include "dplist.h"
#include <stdlib.h>
#include <assert.h>

// Define struct
struct dplist_node {
    void *element;
    struct dplist_node *prev;
    struct dplist_node *next;
};

// Define struct for the list
struct dplist {
    dplist_node_t *head;
    void *(*element_copy)(void *element);
    void (*element_free)(void **element);
    int (*element_compare)(void *x, void *y);
};

dplist_t *dpl_create(
    void *(*element_copy)(void *element),
    void (*element_free)(void **element),
    int (*element_compare)(void *x, void *y)) {
    dplist_t *list = malloc(sizeof(dplist_t));
    assert(list != NULL);
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

void dpl_free(dplist_t **list, bool free_element) {
    assert(list != NULL && *list != NULL);
    dplist_node_t *current = (*list)->head;
    while (current != NULL) {
        dplist_node_t *temp = current;
        current = current->next;
        if (free_element && (*list)->element_free != NULL) {
            (*list)->element_free(&(temp->element));
        }
        free(temp);
    }
    free(*list);
    *list = NULL;
}

int dpl_size(dplist_t *list) {
    if (list == NULL) return -1;
    int size = 0;
    dplist_node_t *current = list->head;
    while (current != NULL) {
        size++;
        current = current->next;
    }
    return size;
}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {
    assert(list != NULL);
    dplist_node_t *new_node = malloc(sizeof(dplist_node_t));
    assert(new_node != NULL);
    new_node->element = insert_copy && list->element_copy != NULL ? list->element_copy(element) : element;
    new_node->prev = NULL;
    new_node->next = NULL;

    if (list->head == NULL) {
        list->head = new_node;
    } else if (index <= 0) {
        new_node->next = list->head;
        list->head->prev = new_node;
        list->head = new_node;
    } else {
        dplist_node_t *current = list->head;
        int pos = 0;
        while (current->next != NULL && pos < index - 1) {
            current = current->next;
            pos++;
        }
        new_node->next = current->next;
        if (current->next != NULL) {
            current->next->prev = new_node;
        }
        current->next = new_node;
        new_node->prev = current;
    }
    return list;
}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {
    assert(list != NULL);
    if (list->head == NULL) return list;

    dplist_node_t *current = list->head;
    if (index <= 0) {
        list->head = current->next;
        if (list->head != NULL) {
            list->head->prev = NULL;
        }
    } else {
        int pos = 0;
        while (current->next != NULL && pos < index) {
            current = current->next;
            pos++;
        }
        if (current->prev != NULL) {
            current->prev->next = current->next;
        }
        if (current->next != NULL) {
            current->next->prev = current->prev;
        }
    }

    if (free_element && list->element_free != NULL) {
        list->element_free(&(current->element));
    }
    free(current);
    return list;
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    if (list == NULL || list->head == NULL) return NULL;
    dplist_node_t *current = list->head;
    int pos = 0;
    while (current->next != NULL && pos < index) {
        current = current->next;
        pos++;
    }
    return current;
}

void *dpl_get_element_at_index(dplist_t *list, int index) {
    dplist_node_t *node = dpl_get_reference_at_index(list, index);
    return node == NULL ? NULL : node->element;
}

int dpl_get_index_of_element(dplist_t *list, void *element) {
    if (list == NULL || list->head == NULL) return -1;
    int index = 0;
    dplist_node_t *current = list->head;
    while (current != NULL) {
        if (list->element_compare != NULL && list->element_compare(current->element, element) == 0) {
            return index;
        }
        current = current->next;
        index++;
    }
    return -1;
}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {
    if (list == NULL || reference == NULL) return NULL;
    dplist_node_t *current = list->head;
    while (current != NULL) {
        if (current == reference) {
            return current->element;
        }
        current = current->next;
    }
    return NULL;
}
