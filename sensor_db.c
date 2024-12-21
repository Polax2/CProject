//
// Created by polan on 20/12/2024.
//
#include "sensor_db.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static FILE *db_file = NULL;

// Initialize the database (or storage file)
void sensor_db_init(const char *filename) {
    db_file = fopen(filename, "w");  // Open file in write mode
    if (db_file == NULL) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // Write the CSV header to the file
    fprintf(db_file, "SensorID,Value,Timestamp\n");
    fflush(db_file);  // Ensure the header is written immediately
    printf("Sensor DB initialized: %s\n", filename);
}

// Write sensor data to the database (or file)
void sensor_db_write(sensor_data_t *data) {
    if (db_file == NULL) {
        fprintf(stderr, "Error: Database not initialized\n");
        return;
    }

    // Log the sensor data to the CSV file
    fprintf(db_file, "%hu,%.2f,%ld\n", data->id, data->value, (long)data->ts);
    fflush(db_file);  // Flush to ensure the data is written immediately
    printf("Sensor DB: Data written (ID: %hu, Value: %.2f, Timestamp: %ld)\n",
           data->id, data->value, (long)data->ts);
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
