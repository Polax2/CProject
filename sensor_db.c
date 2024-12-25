//
// Created by polan on 20/12/2024.
//

#include "sensor_db.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

static FILE *db_file = NULL;
static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

// Initialize the database (or storage file)
void sensor_db_init(const char *filename) {
    db_file = fopen(filename, "w");
    if (db_file == NULL) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(db_file, "SensorID,Value,Timestamp\n");
    fflush(db_file);
    printf("Sensor DB initialized: %s\n", filename);
}

// Write sensor data to the database (CSV file)
void sensor_db_write(sensor_data_t *data) {
    if (db_file == NULL) {
        fprintf(stderr, "Error: Database not initialized\n");
        return;
    }

    pthread_mutex_lock(&db_mutex);
    fprintf(db_file, "%hu,%.2f,%ld\n", data->id, data->value, (long)data->ts);
    fflush(db_file);
    pthread_mutex_unlock(&db_mutex);
}

// Write error logs to the database
void sensor_db_log_error(const char *message) {
    if (db_file == NULL) {
        fprintf(stderr, "Error: Cannot write to database log\n");
        return;
    }
    time_t now = time(NULL);
    fprintf(db_file, "ERROR,,%ld,%s\n", (long)now, message);
    fflush(db_file);
    fprintf(stderr, "Sensor DB Log Error: %s\n", message);
}

// Close the database (or storage file)
void sensor_db_close() {
    if (db_file != NULL) {
        fclose(db_file);
        db_file = NULL;
        printf("Sensor DB closed\n");
    }
}
