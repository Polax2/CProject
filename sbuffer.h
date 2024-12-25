#ifndef SBUFFER_H
#define SBUFFER_H

#include "config.h"
#include <pthread.h>

/* Sbuffer data structure */
typedef struct sbuffer sbuffer_t;

/* Return values for sbuffer operations */
#define SBUFFER_SUCCESS 0
#define SBUFFER_FAILURE -1

/**
 * Initializes the shared buffer.
 * @param buffer Pointer to the buffer to initialize.
 * @return SBUFFER_SUCCESS on success, SBUFFER_FAILURE on failure.
 */
int sbuffer_init(sbuffer_t **buffer);

/**
 * Frees the shared buffer and all associated resources.
 * @param buffer Pointer to the buffer to free.
 * @return SBUFFER_SUCCESS on success, SBUFFER_FAILURE on failure.
 */
int sbuffer_free(sbuffer_t **buffer);

/**
 * Inserts data into the shared buffer.
 * @param buffer Pointer to the buffer.
 * @param data Sensor data to insert.
 * @return SBUFFER_SUCCESS on success, SBUFFER_FAILURE on failure.
 */
int sbuffer_insert(sbuffer_t *buffer, const sensor_data_t *data);

/**
 * Removes data from the shared buffer.
 * @param buffer Pointer to the buffer.
 * @param data Sensor data to fetch.
 * @return SBUFFER_SUCCESS on success, SBUFFER_FAILURE if buffer is empty.
 */
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data);

#endif // SBUFFER_H
