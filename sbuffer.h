#ifndef SBUFFER_H
#define SBUFFER_H

#include <pthread.h>
#include "config.h"

// Internal node structure for FIFO buffer
typedef struct sbuffer_node {
    sensor_data_t data;
    struct sbuffer_node *next;
} sbuffer_node_t;

// Buffer structure
typedef struct sbuffer {
    sbuffer_node_t *head;
    sbuffer_node_t *tail;
    pthread_mutex_t mutex;
} sbuffer_t;

// Buffer operations
int sbuffer_init(sbuffer_t **buffer);
int sbuffer_insert(sbuffer_t *buffer, const sensor_data_t *data);
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data);
void sbuffer_free(sbuffer_t *buffer);

#define SBUFFER_SUCCESS 0
#define SBUFFER_FAILURE -1
#define SBUFFER_NO_DATA -2

#endif // SBUFFER_H
