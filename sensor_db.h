//
// Created by polan on 20/12/2024.
//

#ifndef SENSOR_DB_H
#define SENSOR_DB_H

#include "sbuffer.h"


typedef struct {
    sbuffer_t *buffer;
    int pipe;
} sensor_db_args_t;

void sensor_db_init(const char *filename);

void *sensor_db_process(void *args);

void sensor_db_write(sensor_data_t *data);

void sensor_db_cleanup(void);

#endif // SENSOR_DB_H
