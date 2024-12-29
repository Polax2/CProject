#include "connmgr.h"
#include "sbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include "lib/tcpsock.h"

extern int log_pipe_fd;
extern pthread_mutex_t pipe_mutex;

static void *client_handler(void *arg);

// Main connection manager listen function
void connmgr_listen(struct connmgr_args *args) {
    tcpsock_t *server;
    int result = tcp_passive_open(&server, args->port);

    if (result != TCP_NO_ERROR) {
        log_event("ERROR: Failed to open server socket on port %d", args->port);
        pthread_exit(NULL);
    }

    log_event("INFO: Connection Manager started on port %d", args->port);

    while (1) {
        tcpsock_t *client;
        result = tcp_wait_for_connection(server, &client);
        if (result == TCP_NO_ERROR) {
            log_event("INFO: New client connected");

            struct connmgr_args *client_args = malloc(sizeof(struct connmgr_args));
            *client_args = *args;  // Copy existing args
            client_args->client = client;

            pthread_t client_thread;
            pthread_create(&client_thread, NULL, client_handler, (void *)client_args);
            pthread_detach(client_thread);  // Auto clean-up of thread after exit
        } else {
            log_event("ERROR: Failed to accept client connection");
        }
    }

    tcp_close(&server);
}

// Client handler thread function
static void *client_handler(void *arg) {
    struct connmgr_args *args = (struct connmgr_args *)arg;
    tcpsock_t *client = args->client;
    sensor_data_t data;
    int buf_size = sizeof(sensor_data_t);  // Fix: Use int to store buffer size

    while (1) {
        int result = tcp_receive(client, &data, &buf_size);  // Pass address of buf_size

        if (result == TCP_NO_ERROR && buf_size == sizeof(sensor_data_t)) {
            log_event("INFO: Received data - ID: %hu, Value: %.2f, Timestamp: %ld",
                      data.id, data.value, data.ts);

            pthread_mutex_lock(args->mutex);
            sbuffer_insert(args->buffer, &data);
            pthread_mutex_unlock(args->mutex);
            pthread_cond_signal(args->cond);  // Signal data is available
        } else if (result == TCP_CONNECTION_CLOSED) {
            log_event("INFO: Client disconnected");
            break;
        } else {
            log_event("ERROR: Failed to receive data from client");
        }
    }

    tcp_close(&client);
    free(args);  // Free client_args memory
    pthread_exit(NULL);
}

// Log event function to send logs to the pipe
void log_event(const char *format, ...) {
    va_list args;
    char buffer[256];

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    pthread_mutex_lock(&pipe_mutex);
    write(log_pipe_fd, buffer, strlen(buffer) + 1);  // Write log to pipe
    pthread_mutex_unlock(&pipe_mutex);
}
