//
// Created by polan on 20/12/2024.
//

#include "sensor_db.h"
#include "sbuffer.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Database file handle
static FILE *db_file = NULL;

// Initialize the database file
void sensor_db_init(const char *filename) {
    db_file = fopen(filename, "w");  // Open file in write mode
    if (!db_file) {
        fprintf(stderr, "ERROR: Cannot create file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    fprintf(db_file, "SensorID,Value,Timestamp\n");
    fflush(db_file);
    printf("Sensor DB initialized: %s\n", filename);
}

// Write sensor data to the database
void sensor_db_write(sensor_data_t *data) {
    if (!db_file) {
        fprintf(stderr, "ERROR: Database not initialized\n");
        return;
    }

    fprintf(db_file, "%hu,%.2f,%ld\n", data->id, data->value, (long)data->ts);
    fflush(db_file);
    printf("Sensor DB: Data written (ID: %hu, Value: %.2f, Timestamp: %ld)\n",
           data->id, data->value, (long)data->ts);
}

// Log errors to the database file
void sensor_db_log_error(const char *message) {
    if (!db_file) {
        fprintf(stderr, "ERROR: Cannot write to database log\n");
        return;
    }
    time_t now = time(NULL);
    fprintf(db_file, "ERROR,,%ld,%s\n", (long)now, message);
    fflush(db_file);
    fprintf(stderr, "Sensor DB Log Error: %s\n", message);
}

// Database processing function (runs in thread)
void *sensor_db_process(void *arg) {
    sbuffer_t *buffer = (sbuffer_t *)arg;
    sensor_data_t data;

    while (sbuffer_remove(buffer, &data) == SBUFFER_SUCCESS) {
        sensor_db_write(&data);
    }

    return NULL;
}

// Close the database file
void sensor_db_close() {
    if (db_file) {
        fclose(db_file);
        db_file = NULL;
        printf("Sensor DB closed\n");
    }
}
