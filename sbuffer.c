//
// Created by polan on 20/12/2024.
//

#include "sbuffer.h"
#include <stdlib.h>
#include <pthread.h>

typedef struct sbuffer_node {
    sensor_data_t data;
    struct sbuffer_node *next;
} sbuffer_node_t;

struct sbuffer {
    sbuffer_node_t *head;
    sbuffer_node_t *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

// Initializes the shared buffer
int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;

    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    pthread_mutex_init(&(*buffer)->mutex, NULL);
    pthread_cond_init(&(*buffer)->cond, NULL);

    return SBUFFER_SUCCESS;
}

// Frees the shared buffer
int sbuffer_free(sbuffer_t **buffer) {
    if (buffer == NULL || *buffer == NULL) return SBUFFER_FAILURE;

    sbuffer_node_t *current = (*buffer)->head;
    while (current) {
        sbuffer_node_t *next = current->next;
        free(current);
        current = next;
    }

    pthread_mutex_destroy(&(*buffer)->mutex);
    pthread_cond_destroy(&(*buffer)->cond);
    free(*buffer);
    *buffer = NULL;

    return SBUFFER_SUCCESS;
}

// Inserts data into the buffer (thread-safe)
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    if (buffer == NULL || data == NULL) return SBUFFER_FAILURE;

    sbuffer_node_t *new_node = malloc(sizeof(sbuffer_node_t));
    if (new_node == NULL) return SBUFFER_FAILURE;

    new_node->data = *data;
    new_node->next = NULL;

    pthread_mutex_lock(&buffer->mutex);

    if (buffer->tail) {
        buffer->tail->next = new_node;
    } else {
        buffer->head = new_node;
    }
    buffer->tail = new_node;

    pthread_cond_signal(&buffer->cond);
    pthread_mutex_unlock(&buffer->mutex);

    return SBUFFER_SUCCESS;
}

// Removes data from the buffer (thread-safe)
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    if (buffer == NULL || data == NULL) return SBUFFER_FAILURE;

    pthread_mutex_lock(&buffer->mutex);

    while (buffer->head == NULL) {
        pthread_cond_wait(&buffer->cond, &buffer->mutex);
    }

    sbuffer_node_t *node = buffer->head;
    *data = node->data;
    buffer->head = node->next;
    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }

    free(node);
    pthread_mutex_unlock(&buffer->mutex);

    return SBUFFER_SUCCESS;
}

