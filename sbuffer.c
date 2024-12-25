#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sbuffer.h"
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

// Initialize the shared buffer
int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (!(*buffer)) return SBUFFER_FAILURE;

    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;

    pthread_mutex_init(&(*buffer)->mutex, NULL);
    pthread_cond_init(&(*buffer)->cond, NULL);

    return SBUFFER_SUCCESS;
}

// Free the shared buffer
int sbuffer_free(sbuffer_t **buffer) {
    if (!(*buffer)) return SBUFFER_FAILURE;

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

int sbuffer_insert(sbuffer_t *buffer, const sensor_data_t *data) {
    sbuffer_node_t *new_node = malloc(sizeof(sbuffer_node_t));
    if (!new_node) return SBUFFER_FAILURE;

    memcpy(&new_node->data, data, sizeof(sensor_data_t));
    new_node->next = NULL;

    pthread_mutex_lock(&buffer->mutex);

    if (buffer->tail) {
        buffer->tail->next = new_node;
    } else {
        buffer->head = new_node;
    }
    buffer->tail = new_node;

    printf("Inserted into buffer: SensorID = %hu, Value = %.2f\n",
            data->id, data->value);

    pthread_cond_signal(&buffer->cond);
    pthread_mutex_unlock(&buffer->mutex);

    return SBUFFER_SUCCESS;
}


// Remove data from the buffer
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    pthread_mutex_lock(&buffer->mutex);

    while (!buffer->head) {
        pthread_cond_wait(&buffer->cond, &buffer->mutex);
    }

    sbuffer_node_t *old_head = buffer->head;
    memcpy(data, &old_head->data, sizeof(sensor_data_t));

    buffer->head = old_head->next;
    if (!buffer->head) buffer->tail = NULL;

    free(old_head);

    pthread_mutex_unlock(&buffer->mutex);

    return SBUFFER_SUCCESS;
}
