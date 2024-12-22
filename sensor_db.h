//
// Created by polan on 20/12/2024.
//

#ifndef SENSOR_DB_H
#define SENSOR_DB_H

#include "config.h"

// Initializes the sensor database
void sensor_db_init(const char *filename);

// Writes sensor data to the database
void sensor_db_write(sensor_data_t *data);

// Logs error messages to the database
void sensor_db_log_error(const char *message);

// Closes the database file
void sensor_db_close();

#endif // SENSOR_DB_H

