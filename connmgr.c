#include "connmgr.h"
#include "config.h"
#include "sbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Define pipe and mutex for logging (extern from main)
extern int log_pipe_fd[2];
extern pthread_mutex_t pipe_mutex;

#define MOCK_SENSOR_COUNT 5
#define MOCK_SLEEP_TIME 2
#define MOCK_CLIENTS 2  // Simulate two client connections

// Log function to ensure proper thread-safe logging to pipe
void log_with_buffer_state(const char *msg, sbuffer_t *buffer) {
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "%s | Buffer address: %p \n", msg, (void *)buffer);
    log_to_logger(log_msg);
}

// Generate random mock sensor data
sensor_data_t generate_mock_data() {
    sensor_data_t data;
    data.id = rand() % 100 + 1;
    data.value = 15.0 + (rand() % 100) / 10.0;
    data.ts = time(NULL);
    return data;
}



// Simulate client connections
void *client_handler(void *arg) {
    connmgr_args_t *client_args = (connmgr_args_t *)arg;
    sbuffer_t *buffer = client_args->buffer;

    // Simulated sensor data (mock sensor ID for example)
    sensor_data_t data = { .id = rand() % 100, .value = 20.0, .ts = time(NULL) };

    char msg[256];
    snprintf(msg, sizeof(msg), "Sensor node %d has opened a new connection \n", data.id);
    log_to_logger(msg);  // Log new connection

    for (int i = 0; i < 5; i++) {  // Simulate sensor sending 5 data points
        sbuffer_insert(buffer, &data);
        snprintf(msg, sizeof(msg), "Data inserted (ID: %d, Value: %.2f) \n", data.id, data.value);
        log_to_logger(msg);
        sleep(2);  // Simulate delay between data points
    }

    snprintf(msg, sizeof(msg), "Sensor node %d has closed the connection \n", data.id);
    log_to_logger(msg);  // Log disconnection

    free(client_args);
    pthread_exit(NULL);
}

// Listen for new connections
void connmgr_listen(void *args) {
    connmgr_args_t *connmgr_args = (connmgr_args_t *)args;
    sbuffer_t *buffer = connmgr_args->buffer;


    log_to_logger("Connection Manager started. \n");

    for (int i = 0; i < 2; i++) {  // Simulate 2 sensor node connections
        connmgr_args_t *client_args = malloc(sizeof(connmgr_args_t));
        if (!client_args) {
            log_to_logger("Failed to allocate memory for client_args.\n");
            continue;
        }

        client_args->buffer = buffer;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler, client_args) != 0) {
            log_to_logger("Failed to create client thread.\n");
            free(client_args);
            continue;
        }
        pthread_detach(client_thread);  // Allow thread to exit independently
        sleep(5);  // Simulate time between new client connections
    }

    log_to_logger("Connection Manager finished accepting clients.\n");
}

