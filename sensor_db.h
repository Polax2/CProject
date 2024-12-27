#ifndef SENSOR_DB_H
#define SENSOR_DB_H

#include "sbuffer.h"

void sensor_db_init(const char *filename);
void *sensor_db_process(void *buffer);
void sensor_db_cleanup();
void log_event(const char *format, ...);

#endif /* SENSOR_DB_H */
