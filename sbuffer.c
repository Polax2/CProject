#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "sbuffer.h"
#include "config.h"

int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;

    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    pthread_mutex_init(&(*buffer)->mutex, NULL);
    return SBUFFER_SUCCESS;
}

// Insert data into buffer (FIFO)
int sbuffer_insert(sbuffer_t *buffer, const sensor_data_t *data) {
    sbuffer_node_t *new_node = malloc(sizeof(sbuffer_node_t));
    if (new_node == NULL) return SBUFFER_FAILURE;

    memcpy(&new_node->data, data, sizeof(sensor_data_t));
    new_node->next = NULL;

    pthread_mutex_lock(&buffer->mutex);
    if (buffer->tail == NULL) {
        buffer->head = buffer->tail = new_node;
    } else {
        buffer->tail->next = new_node;
        buffer->tail = new_node;
    }
    pthread_mutex_unlock(&buffer->mutex);
    return SBUFFER_SUCCESS;
}

// Remove data from buffer (FIFO)
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    pthread_mutex_lock(&buffer->mutex);

    if (buffer->head == NULL) {
        pthread_mutex_unlock(&buffer->mutex);
        return SBUFFER_NO_DATA;  // Return no data available
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

// Free buffer (now returns void)
void sbuffer_free(sbuffer_t *buffer) {
    pthread_mutex_lock(&buffer->mutex);

    sbuffer_node_t *current = buffer->head;
    while (current != NULL) {
        sbuffer_node_t *next = current->next;
        free(current);
        current = next;
    }

    pthread_mutex_unlock(&buffer->mutex);
    pthread_mutex_destroy(&buffer->mutex);
    free(buffer);
}
