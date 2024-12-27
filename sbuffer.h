#ifndef SBUFFER_H
#define SBUFFER_H

#include "config.h"
#include <pthread.h>

typedef struct sbuffer sbuffer_t;

// Initialize buffer
int sbuffer_init(sbuffer_t **buffer);

// Insert data into buffer
int sbuffer_insert(sbuffer_t *buffer, const sensor_data_t *data);

// Remove data from buffer
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data);

// Free buffer
void sbuffer_free(sbuffer_t *buffer);  // Fixed to match sbuffer.c (void return)

// Status codes
#define SBUFFER_SUCCESS 0
#define SBUFFER_FAILURE -1
#define SBUFFER_NO_DATA 1  // Add this to align with the error handling in sbuffer.c

#endif // SBUFFER_H
