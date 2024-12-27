//
// Created by polan on 20/12/2024.
//

#ifndef DATAMGR_H
#define DATAMGR_H

#include "config.h"
#include "sbuffer.h"

// Process sensor data from the shared buffer
void *datamgr_process(void *buffer);
void log_event(const char *format, ...);

#endif /* DATAMGR_H */
