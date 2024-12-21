//
// Created by polan on 20/12/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include "connmgr.h"
#include "datamgr.h"
#include "sbuffer.h"
#include "sensor_db.h"

#define LOG_MSG_SIZE 256

int main() {
    sbuffer_t *buffer = NULL;

    // Initialize the shared buffer
    if (sbuffer_init(&buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize shared buffer\n");
        return EXIT_FAILURE;
    }

    // Initialize data manager and sensor database
    datamgr_init("room_sensor.map");
    sensor_db_init("data.csv");

    // Start the connection manager to listen for incoming sensor nodes
    connmgr_listen(12345, 5, buffer);

    // Process data from buffer (loop to keep processing data until termination)
    sensor_data_t data;
    while (sbuffer_remove(buffer, &data) == SBUFFER_SUCCESS) {
        // Pass data to the data manager for processing (e.g., running averages)
        datamgr_process(buffer);

        // Write data to persistent storage
        sensor_db_write(&data);
    }

    // Cleanup resources
    datamgr_free();
    sensor_db_close();
    sbuffer_free(&buffer);

    printf("Sensor gateway shutting down\n");
    return EXIT_SUCCESS;
}
