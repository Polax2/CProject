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
    snprintf(log_msg, sizeof(log_msg), "%s | Buffer address: %p", msg, (void *)buffer);
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

// Client handler to simulate sensor node communication
static void *client_handler(void *arg) {
    connmgr_args_t *client_args = (connmgr_args_t *)arg;
    sbuffer_t *buffer = client_args->buffer;
    char msg[256];

    if (buffer == NULL) {
        log_with_buffer_state("Client handler started with NULL buffer.", buffer);
        free(client_args);
        pthread_exit(NULL);
    }

    snprintf(msg, sizeof(msg), "Client thread started.");
    log_with_buffer_state(msg, buffer);

    for (int i = 0; i < MOCK_SENSOR_COUNT; i++) {
        sensor_data_t data = generate_mock_data();
        int result = sbuffer_insert(buffer, &data);

        if (result == SBUFFER_SUCCESS) {
            snprintf(msg, sizeof(msg), "Data inserted (ID: %d, Value: %.2f)", data.id, data.value);
            log_with_buffer_state(msg, buffer);
        } else {
            log_with_buffer_state("Buffer insertion failed in client handler.", buffer);
        }

        sleep(MOCK_SLEEP_TIME);  // Simulate delay between sensor readings
    }

    snprintf(msg, sizeof(msg), "Client thread finished.");
    log_with_buffer_state(msg, buffer);

    free(client_args);  // Free the argument struct
    pthread_exit(NULL);
}

// Listen for new connections (mock two clients)
void connmgr_listen(connmgr_args_t *args) {
    sbuffer_t *buffer = args->buffer;
    char msg[256];

    if (buffer == NULL) {
        log_with_buffer_state("Connection Manager started with NULL buffer.", buffer);
        pthread_exit(NULL);
    }

    snprintf(msg, sizeof(msg), "Connection Manager started.");
    log_with_buffer_state(msg, buffer);

    for (int i = 0; i < MOCK_CLIENTS; i++) {
        connmgr_args_t *client_args = malloc(sizeof(connmgr_args_t));
        if (client_args == NULL) {
            log_to_logger("Failed to allocate memory for client_args.");
            continue;
        }
        client_args->buffer = buffer;

        if (client_args->buffer == NULL) {
            log_with_buffer_state("client_args initialized with NULL buffer.", buffer);
        }

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler, client_args) != 0) {
            log_with_buffer_state("Failed to create client thread.", buffer);
            free(client_args);
            continue;
        }
        pthread_detach(client_thread);

        snprintf(msg, sizeof(msg), "Client %d connected.", i + 1);
        log_with_buffer_state(msg, buffer);

        sleep(5);  // Simulate delay between new client connections
    }

    snprintf(msg, sizeof(msg), "Connection Manager finished accepting clients.");
    log_with_buffer_state(msg, buffer);
}
