//
// Created by polan on 20/12/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include "connmgr.h"
#include "sbuffer.h"
#include "datamgr.h"
#include "sbuffer.h"
#include "sensor_db.h"

int main() {
    sbuffer_t *buffer = NULL;

    // Initialize the shared buffer
    if (sbuffer_init(&buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize shared buffer\n");
        return EXIT_FAILURE;
    }

    // Initialize the data manager
    datamgr_init("room_sensor.map");

    // Initialize the database
    sensor_db_init("data.csv");

    // Start the connection manager
    connmgr_listen(12345, 5, buffer);

    // Process data in a loop (or separate threads if applicable)
    datamgr_process(buffer);

    // Example: Retrieve data and write it to the database
    sensor_data_t data;
    while (sbuffer_remove(buffer, &data) == SBUFFER_SUCCESS) {
        sensor_db_write(&data);
    }

    // Free resources
    datamgr_free();
    sensor_db_close();
    sbuffer_free(&buffer);

    return EXIT_SUCCESS;
}
