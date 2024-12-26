//
// Created by polan on 20/12/2024.
//

#ifndef DATAMGR_H
#define DATAMGR_H

#include "config.h"
#include "sbuffer.h"

/**
 * Initialize the data manager with the room-sensor mapping file and log pipe
 */
void datamgr_init(const char *map_file, int log_pipe);

/**
 * Process data from the shared buffer
 */
void *datamgr_process(void *buffer);

/**
 * Free all resources used by the data manager
 */
void datamgr_free();

#endif /* DATAMGR_H */
