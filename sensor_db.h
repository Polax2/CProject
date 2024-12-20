//
// Created by polan on 20/12/2024.
//

#ifndef SENSOR_DB_H_
#define SENSOR_DB_H_

#include "config.h"
#include "sbuffer.h"

// Initialize the database (or storage file)
void sensor_db_init(const char *filename);

// Write sensor data to the database (or file)
void sensor_db_write(sensor_data_t *data);

// Close the database (or storage file)
void sensor_db_close();

#endif // SENSOR_DB_H_

