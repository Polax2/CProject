#include "connmgr.h"
#include "config.h"
#include "sbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

extern int log_pipe_fd[2];
extern pthread_mutex_t pipe_mutex;

#define MOCK_SENSOR_COUNT 5
#define MOCK_SLEEP_TIME 2



// Generates mock sensor data
sensor_data_t generate_mock_data() {
    sensor_data_t data;
    data.id = rand() % 100 + 1;
    data.value = 15.0 + (rand() % 100) / 10.0;
    data.ts = time(NULL);
    return data;
}

// Simulates client handling by generating mock data and inserting it into the buffer
static void *client_handler(void *arg) {
    struct connmgr_args *client_args = (struct connmgr_args *)arg;
    sbuffer_t *buffer = client_args->buffer;
    char msg[256];

    for (int i = 0; i < MOCK_SENSOR_COUNT; i++) {
        sensor_data_t data = generate_mock_data();

        if (sbuffer_insert(buffer, &data) == SBUFFER_SUCCESS) {
            snprintf(msg, sizeof(msg), "Mock Data inserted (ID: %d, Value: %.2f)", data.id, data.value);
            log_to_logger(msg);
        } else {
            log_to_logger("Buffer insertion failed.");
        }

        sleep(MOCK_SLEEP_TIME);
    }

    snprintf(msg, sizeof(msg), "Mock client handler finished.");
    log_to_logger(msg);

    free(client_args);
    pthread_exit(NULL);
}

// Connection Manager Listen Function - Simulating Client Connections
void connmgr_listen(struct connmgr_args *args) {
    char msg[256];
    snprintf(msg, sizeof(msg), "Connection Manager started.");
    log_to_logger(msg);

    while (1) {
        struct connmgr_args *client_args = malloc(sizeof(struct connmgr_args));
        if (!client_args) {
            log_to_logger("Failed to allocate memory for mock client.");
            continue;
        }

        *client_args = *args;  // Copy shared buffer and sync objects

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, client_handler, (void *)client_args);
        pthread_detach(client_thread);

        sleep(5);  // Simulate delay between new client connections
    }
}
