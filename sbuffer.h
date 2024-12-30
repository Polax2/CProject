#ifndef SBUFFER_H
#define SBUFFER_H

#include "config.h"
#include <pthread.h>
#include <stdbool.h>

// Forward declarations
typedef struct sbuffer sbuffer_t;
typedef struct sbuffer_node sbuffer_node_t;

// Sbuffer structure definition
struct sbuffer {
    sbuffer_node_t *head;
    sbuffer_node_t *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool end_flag;
};


sbuffer_t *sbuffer_init();
int sbuffer_free(sbuffer_t *buffer);
int sbuffer_insert(sbuffer_t *buffer, const sensor_data_t *data);
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data);
void sbuffer_terminate(sbuffer_t *buffer);
bool sbuffer_is_terminated(sbuffer_t *buffer);

// Return codes
#define SBUFFER_SUCCESS 0
#define SBUFFER_FAILURE -1
#define SBUFFER_NO_DATA 1

#endif /* SBUFFER_H */
