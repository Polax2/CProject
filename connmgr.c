#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/select.h>
#include "connmgr.h"
#include "lib/tcpsock.h"
#include "sbuffer.h"

static int active_clients = 0;
static int max_clients;
static int timeout;

void log_event(int log_fd, const char *message, pthread_mutex_t *pipe_mutex) {
    pthread_mutex_lock(pipe_mutex);
    write(log_fd, message, strlen(message) + 1);
    pthread_mutex_unlock(pipe_mutex);
}

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
        snprintf(log_message, sizeof(log_message), "Failed to get socket descriptor.");
        log_event(log_fd, log_message, pipe_mutex);
        free(resources);
        return NULL;
    }

    snprintf(log_message, sizeof(log_message), "Sensor node connected (sd: %d).", client_sock);
    log_event(log_fd, log_message, pipe_mutex);

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(client_sock, &readfds);
        struct timeval tv = {resources->args[2], 0};  // Timeout
        int retval = select(client_sock + 1, &readfds, NULL, NULL, &tv);

        if (retval == 0) {
            snprintf(log_message, sizeof(log_message), "Timeout (sd: %d).", client_sock);
            log_event(log_fd, log_message, pipe_mutex);
            break;
        }

        bytes = sizeof(data.id);
        if (tcp_receive(client, (void *)&data.id, &bytes) != TCP_NO_ERROR || bytes == 0) break;
        bytes = sizeof(data.value);
        if (tcp_receive(client, (void *)&data.value, &bytes) != TCP_NO_ERROR || bytes == 0) break;
        bytes = sizeof(data.ts);
        if (tcp_receive(client, (void *)&data.ts, &bytes) != TCP_NO_ERROR || bytes == 0) break;

        snprintf(log_message, sizeof(log_message), "Received data: ID=%" PRIu16 ", Value=%.2f", data.id, data.value);
        log_event(log_fd, log_message, pipe_mutex);

        pthread_mutex_lock(mutex);
        sbuffer_insert(buffer, &data);
        pthread_cond_signal(cond);
        pthread_mutex_unlock(mutex);
    }

    tcp_close(&client);
    pthread_mutex_lock(mutex);
    active_clients--;
    pthread_mutex_unlock(mutex);

    snprintf(log_message, sizeof(log_message), "Connection closed (sd: %d).", client_sock);
    log_event(log_fd, log_message, pipe_mutex);

    free(resources);
    return NULL;
}

void connmgr_listen(struct connmgr_args *args) {
    int port = args->args[0];
    max_clients = args->args[1];
    timeout = args->args[2];

    tcpsock_t *server;
    if (tcp_passive_open(&server, port) != TCP_NO_ERROR) {
        fprintf(stderr, "Failed to open TCP server.\n");
        exit(EXIT_FAILURE);
    }

    char log_message[128];
    snprintf(log_message, sizeof(log_message), "Server listening on port %d.", port);
    log_event(args->log_fd, log_message, args->pipe_mutex);

    while (active_clients < max_clients) {
        tcpsock_t *client;
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) continue;

        pthread_t client_thread;
        struct connmgr_args *resources = malloc(sizeof(*resources));
        *resources = *args;
        resources->client = client;

        pthread_mutex_lock(args->mutex);
        active_clients++;
        pthread_mutex_unlock(args->mutex);

        pthread_create(&client_thread, NULL, client_handler, resources);
        pthread_detach(client_thread);
    }

    tcp_close(&server);
}
