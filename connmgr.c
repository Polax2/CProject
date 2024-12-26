//
// Created by polan on 20/12/2024.
//

#include "connmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/select.h>
#include "lib/tcpsock.h"
#include "sbuffer.h"

// Global variables for client tracking
static int active_clients = 0;

// Helper function to log events to the pipe
void log_event(int log_fd, const char *message, pthread_mutex_t *pipe_mutex) {
    pthread_mutex_lock(pipe_mutex);
    write(log_fd, message, strlen(message) + 1); // Include null terminator
    pthread_mutex_unlock(pipe_mutex);
}

// Function to handle client connections
void *client_handler(void *arg) {
    struct connmgr_args *resources = (struct connmgr_args *)arg;

    tcpsock_t *client = resources->client;
    sbuffer_t *buffer = resources->buffer;
    pthread_mutex_t *mutex = resources->mutex;
    pthread_cond_t *cond = resources->cond;
    int log_fd = resources->log_fd;
    pthread_mutex_t *pipe_mutex = resources->pipe_mutex;

    sensor_data_t data;
    int bytes;
    char log_message[128];
    int client_sock;

    if (tcp_get_sd(client, &client_sock) != TCP_NO_ERROR) {
        snprintf(log_message, sizeof(log_message), "Failed to get socket descriptor for client.");
        log_event(log_fd, log_message, pipe_mutex);
        free(resources);
        return NULL;
    }

    snprintf(log_message, sizeof(log_message), "Sensor node connected (socket descriptor: %d).", client_sock);
    log_event(log_fd, log_message, pipe_mutex);

    while (1) {
        // Use select() to implement a timeout for inactivity
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(client_sock, &readfds);

        struct timeval tv = {resources->args[2], 0}; // Timeout in seconds
        int retval = select(client_sock + 1, &readfds, NULL, NULL, &tv);

        if (retval == 0) { // Timeout
            snprintf(log_message, sizeof(log_message), "Sensor node timed out (socket descriptor: %d).", client_sock);
            log_event(log_fd, log_message, pipe_mutex);
            break;
        } else if (retval < 0) { // Error
            perror("select() error");
            break;
        }

        // Receive sensor ID
        bytes = sizeof(data.id);
        if (tcp_receive(client, (void *)&data.id, &bytes) != TCP_NO_ERROR || bytes == 0) {
            snprintf(log_message, sizeof(log_message), "Sensor node disconnected (socket descriptor: %d).", client_sock);
            log_event(log_fd, log_message, pipe_mutex);
            break;
        }

        // Receive sensor value
        bytes = sizeof(data.value);
        if (tcp_receive(client, (void *)&data.value, &bytes) != TCP_NO_ERROR || bytes == 0) {
            snprintf(log_message, sizeof(log_message), "Sensor node disconnected (socket descriptor: %d).", client_sock);
            log_event(log_fd, log_message, pipe_mutex);
            break;
        }

        // Receive timestamp
        bytes = sizeof(data.ts);
        if (tcp_receive(client, (void *)&data.ts, &bytes) != TCP_NO_ERROR || bytes == 0) {
            snprintf(log_message, sizeof(log_message), "Sensor node disconnected (socket descriptor: %d).", client_sock);
            log_event(log_fd, log_message, pipe_mutex);
            break;
        }

        // Log received data
        snprintf(log_message, sizeof(log_message), "Received data: ID=%" PRIu16 ", Value=%.2f, Timestamp=%ld",
                 data.id, data.value, (long)data.ts);
        log_event(log_fd, log_message, pipe_mutex);

        // Write data to shared buffer
        pthread_mutex_lock(mutex);
        sbuffer_insert(buffer, &data);
        pthread_cond_signal(cond);
        pthread_mutex_unlock(mutex);
    }

    tcp_close(&client);
    pthread_mutex_lock(mutex);
    active_clients--;
    pthread_mutex_unlock(mutex);

    snprintf(log_message, sizeof(log_message), "Sensor node connection closed (socket descriptor: %d).", client_sock);
    log_event(log_fd, log_message, pipe_mutex);

    free(resources); // Free the dynamically allocated resources
    return NULL;
}

// Function to initialize and run the connection manager
void connmgr_listen(struct connmgr_args *args) {
    int port = args->args[0];
    int max_clients = args->args[1];
    int timeout = args->args[2];

    (void)max_clients; // Suppress unused variable warning
    (void)timeout;     // Suppress unused variable warning

    tcpsock_t *server;
    if (tcp_passive_open(&server, port) != TCP_NO_ERROR) {
        fprintf(stderr, "Failed to open TCP server on port %d\n", port);
        exit(EXIT_FAILURE);
    }

    char log_message[128];
    snprintf(log_message, sizeof(log_message), "Server started on port %d.", port);
    log_event(args->log_fd, log_message, args->pipe_mutex);

    while (active_clients < max_clients) {
        tcpsock_t *client;
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) {
            fprintf(stderr, "Error waiting for client connection\n");
            continue;
        }

        snprintf(log_message, sizeof(log_message), "New client connected.");
        log_event(args->log_fd, log_message, args->pipe_mutex);

        pthread_t client_thread;
        struct connmgr_args *resources = malloc(sizeof(*resources));
        *resources = *args; // Copy arguments to avoid race conditions
        resources->client = client;

        pthread_mutex_lock(args->mutex);
        active_clients++;
        pthread_mutex_unlock(args->mutex);

        if (pthread_create(&client_thread, NULL, client_handler, resources) != 0) {
            fprintf(stderr, "Failed to create client thread\n");
            tcp_close(&client);
            free(resources);
            pthread_mutex_lock(args->mutex);
            active_clients--;
            pthread_mutex_unlock(args->mutex);
            continue;
        }

        pthread_detach(client_thread);
    }

    snprintf(log_message, sizeof(log_message), "Server shutting down.");
    log_event(args->log_fd, log_message, args->pipe_mutex);
    tcp_close(&server);
}
