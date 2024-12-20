//
// Created by polan on 20/12/2024.
//

#ifndef SBUFFER_H_
#define SBUFFER_H_

#include "config.h"

typedef struct sbuffer sbuffer_t;

// Initializes the shared buffer
int sbuffer_init(sbuffer_t **buffer);

// Frees the shared buffer
int sbuffer_free(sbuffer_t **buffer);

// Inserts data into the buffer (thread-safe)
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data);

// Removes data from the buffer (thread-safe)
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data);

#define SBUFFER_SUCCESS 0
#define SBUFFER_FAILURE -1

#endif // SBUFFER_H_

