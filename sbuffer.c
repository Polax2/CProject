#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include "sbuffer.h"

/**
 * Internal node structure for the buffer.
 */
typedef struct sbuffer_node {
    sensor_data_t data;
    struct sbuffer_node *next;
} sbuffer_node_t;


/**
 * Initializes the buffer and returns the buffer address.
 */
sbuffer_t *sbuffer_init() {
    sbuffer_t *buffer = malloc(sizeof(sbuffer_t));
    if (buffer == NULL) return NULL;

    buffer->head = NULL;
    buffer->tail = NULL;
    buffer->end_flag = false;

    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->cond, NULL);

    return buffer;
}

int sbuffer_free(sbuffer_t *buffer) {
    if (buffer == NULL) return SBUFFER_FAILURE;

    pthread_mutex_lock(&buffer->mutex);
    sbuffer_node_t *tmp = buffer->head;
    while (tmp != NULL) {
        sbuffer_node_t *next = tmp->next;
        free(tmp);
        tmp = next;
    }
    pthread_mutex_unlock(&buffer->mutex);
    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->cond);

    free(buffer);
    return SBUFFER_SUCCESS;
}

/**
 * Inserts data into the buffer.
 */
int sbuffer_insert(sbuffer_t *buffer, const sensor_data_t *data) {
    if (buffer == NULL || data == NULL) return SBUFFER_FAILURE;

    sbuffer_node_t *new_node = malloc(sizeof(sbuffer_node_t));
    if (new_node == NULL) return SBUFFER_FAILURE;

    new_node->data = *data;
    new_node->next = NULL;

    pthread_mutex_lock(&buffer->mutex);

    if (buffer->tail == NULL) {
        buffer->head = new_node;
        buffer->tail = new_node;
    } else {
        buffer->tail->next = new_node;
        buffer->tail = new_node;
    }

    pthread_cond_signal(&buffer->cond);
    pthread_mutex_unlock(&buffer->mutex);

    return SBUFFER_SUCCESS;
}

/**
 * Removes data from the buffer. Returns SBUFFER_NO_DATA if empty.
 */
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    if (buffer == NULL || data == NULL) return SBUFFER_FAILURE;

    pthread_mutex_lock(&buffer->mutex);

    while (buffer->head == NULL) {
        if (buffer->end_flag) {
            pthread_mutex_unlock(&buffer->mutex);
            return SBUFFER_NO_DATA;
        }
        pthread_cond_wait(&buffer->cond, &buffer->mutex);
    }

    sbuffer_node_t *temp = buffer->head;
    *data = temp->data;

    buffer->head = buffer->head->next;
    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }

    free(temp);
    pthread_mutex_unlock(&buffer->mutex);

    return SBUFFER_SUCCESS;
}

/**
 * Terminates the buffer, signaling threads waiting on the condition variable.
 */
void sbuffer_terminate(sbuffer_t *buffer) {
    if (buffer == NULL) return;

    pthread_mutex_lock(&buffer->mutex);
    buffer->end_flag = true;
    pthread_cond_broadcast(&buffer->cond);  // Wake all waiting threads
    pthread_mutex_unlock(&buffer->mutex);
}

/**
 * Checks if the buffer has been terminated.
 */
bool sbuffer_is_terminated(sbuffer_t *buffer) {
    if (buffer == NULL) return true;

    pthread_mutex_lock(&buffer->mutex);
    bool terminated = buffer->end_flag;
    pthread_mutex_unlock(&buffer->mutex);

    return terminated;
}
