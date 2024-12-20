//
// Created by polan on 20/12/2024.
//

#include "sensor_db.h"
#include <stdio.h>
#include <stdlib.h>

static FILE *db_file = NULL;

// Initialize the database (or storage file)
void sensor_db_init(const char *filename) {
    db_file = fopen(filename, "w");
    if (db_file == NULL) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // Write the header to the file
    fprintf(db_file, "SensorID,Value,Timestamp\n");
    fflush(db_file); // Ensure the header is written
    printf("Sensor DB initialized: %s\n", filename);
}

// Write sensor data to the database (or file)
void sensor_db_write(sensor_data_t *data) {
    if (db_file == NULL) {
        fprintf(stderr, "Error: Database not initialized\n");
        return;
    }

    // Write data to the file
    fprintf(db_file, "%hu,%.2f,%ld\n", data->id, data->value, (long)data->ts);
    fflush(db_file); // Ensure data is written immediately
    printf("Sensor DB: Data written (ID: %hu, Value: %.2f, Timestamp: %ld)\n",
           data->id, data->value, (long)data->ts);
}

// Close the database (or storage file)
void sensor_db_close() {
    if (db_file != NULL) {
        fclose(db_file);
        db_file = NULL;
        printf("Sensor DB closed\n");
    }
}

