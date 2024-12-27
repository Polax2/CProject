#include "connmgr.h"
#include "sbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include "lib/tcpsock.h"


extern int log_pipe_fd;  // Use the global log pipe
extern pthread_mutex_t pipe_mutex;

void log_event(const char *format, ...) {
    va_list args;
    char buffer[256];

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    pthread_mutex_lock(&pipe_mutex);
    write(log_pipe_fd, buffer, strlen(buffer) + 1);
    pthread_mutex_unlock(&pipe_mutex);
}

void *client_handler(void *arg) {
    struct connmgr_args *args = (struct connmgr_args *)arg;
    sensor_data_t data;
    int result, bytes;

    log_event("Client connected to port %d", args->port);

    while (1) {
        bytes = sizeof(sensor_data_t);
        result = tcp_receive(args->client, (void *)&data, &bytes);
        if (result == TCP_NO_ERROR) {
            pthread_mutex_lock(args->mutex);
            sbuffer_insert(args->buffer, &data);
            pthread_mutex_unlock(args->mutex);
            pthread_cond_signal(args->cond);

            log_event("Received data - ID: %hu, Value: %.2f, Timestamp: %ld",
                      data.id, data.value, (long)data.ts);
        } else {
            log_event("Client disconnected from port %d", args->port);
            tcp_close(&args->client);
            break;
        }
    }
    return NULL;
}

void connmgr_listen(struct connmgr_args *args) {
    tcpsock_t *server, *client;
    int conn_count = 0;

    if (tcp_passive_open(&server, args->port) != TCP_NO_ERROR) {
        log_event("Failed to open passive connection on port %d", args->port);
        return;
    }

    log_event("Connection Manager listening on port %d", args->port);

    while (conn_count < args->max_clients) {
        if (tcp_wait_for_connection(server, &client) == TCP_NO_ERROR) {
            args->client = client;
            pthread_t client_tid;
            pthread_create(&client_tid, NULL, client_handler, (void *)args);
            conn_count++;
        }
    }

    tcp_close(&server);
    log_event("Connection Manager stopped listening on port %d", args->port);
}
