//
// Created by polan on 20/12/2024.
//
#ifndef DATAMGR_H
#define DATAMGR_H

#include <stdint.h>
#include "sbuffer.h"

/**
 * Initializes the Data Manager.
 * Reads the room-sensor mapping from the file and sets up internal structures.
 *
 * @param map_file Path to the room-sensor mapping file.
 * @return 0 on success, -1 on failure.
 */
int datamgr_init(const char *map_file);

/**
 * Cleans up resources used by the Data Manager.
 */
void datamgr_free();

/**
 * Processes data from the shared buffer.
 * Continuously fetches data, calculates running averages, and checks temperature thresholds.
 *
 * @param buffer Pointer to the shared buffer.
 * @return NULL
 */
void *datamgr_process(void *buffer);

#endif // DATAMGR_H
