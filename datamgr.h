//
// Created by polan on 20/12/2024.
//
#ifndef DATAMGR_H_
#define DATAMGR_H_

#include "sbuffer.h"
#include "config.h"

// Initialize the data manager and load the room-sensor map
void datamgr_init(const char *map_file);

// Process data from the shared buffer
void datamgr_process(sbuffer_t *buffer);

// Free resources used by the data manager
void datamgr_free();

#endif // DATAMGR_H_
