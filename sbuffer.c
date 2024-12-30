#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include "sbuffer.h"
#include "config.h"


// Mark the head node for removal
void sbuffer_mark_for_removal(sbuffer_t *buffer) {
    pthread_mutex_lock(&buffer->mutex);
    if (buffer->head != NULL) {
        buffer->head->end_flag = true;
    }
    pthread_mutex_unlock(&buffer->mutex);
}

// Remove nodes that are marked for deletion
void sbuffer_remove_marked(sbuffer_t *buffer) {
    pthread_mutex_lock(&buffer->mutex);

    while (buffer->head != NULL && buffer->head->end_flag) {
        sbuffer_node_t *temp = buffer->head;
        buffer->head = buffer->head->next;
        free(temp);

        char msg[256];
        snprintf(msg, sizeof(msg), "Node removed from buffer. \n");
        log_to_logger(msg);  // Log node removal through the logger process
    }

    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }

    pthread_mutex_unlock(&buffer->mutex);
}

// Initialize buffer
sbuffer_t *sbuffer_init() {
    sbuffer_t *buffer = malloc(sizeof(sbuffer_t));
    if (buffer == NULL) {
        printf("ERROR: Memory allocation for buffer failed.\n");
        return NULL;
    }

    buffer->head = NULL;
    buffer->tail = NULL;
    buffer->end_flag = false;

    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->cond, NULL);

    printf("Buffer initialized.\n");
    return buffer;
}

// Insert data into buffer
int sbuffer_insert(sbuffer_t *buffer, const sensor_data_t *data) {
    if (!buffer || !data) {
        printf("sbuffer_insert: Buffer or data is NULL.\n");
        return -1;
    }

    sbuffer_node_t *new_node = malloc(sizeof(sbuffer_node_t));
    if (!new_node) {
        printf("sbuffer_insert: Failed to allocate memory for node.\n");
        return -1;
    }

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

    return 0;
}


sensor_data_t *sbuffer_read(sbuffer_t *buffer) {
    if (buffer == NULL) return NULL;

    pthread_mutex_lock(&buffer->mutex);

    while (buffer->head == NULL) {
        if (buffer->end_flag) {
            pthread_mutex_unlock(&buffer->mutex);
            printf("Buffer empty, termination signal received.\n");
            return NULL;
        }
        pthread_cond_wait(&buffer->cond, &buffer->mutex);
    }

    sbuffer_node_t *temp = buffer->head;
    sensor_data_t *data = &temp->data;

    pthread_mutex_unlock(&buffer->mutex);
    return data;
}



// Cleanup buffer nodes
void sbuffer_cleanup(sbuffer_t *buffer) {
    if (buffer == NULL) return;

    pthread_mutex_lock(&buffer->mutex);

    sbuffer_node_t *temp;
    while (buffer->head != NULL) {
        temp = buffer->head;
        buffer->head = buffer->head->next;
        free(temp);
    }
    buffer->tail = NULL;

    pthread_mutex_unlock(&buffer->mutex);
    printf("Buffer cleaned up.\n");
}

// Free buffer and resources
int sbuffer_free(sbuffer_t *buffer) {
    if (buffer == NULL) return SBUFFER_FAILURE;

    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->cond);
    free(buffer);

    printf("Buffer freed.\n");
    return SBUFFER_SUCCESS;
}

// Terminate buffer operations
void sbuffer_terminate(sbuffer_t *buffer) {
    if (buffer == NULL) return;

    pthread_mutex_lock(&buffer->mutex);
    buffer->end_flag = true;
    pthread_cond_broadcast(&buffer->cond);
    pthread_mutex_unlock(&buffer->mutex);

    printf("Buffer termination signaled.\n");
}
