//
// Created by polan on 20/12/2024.
//
#include "connmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>
#include "lib/tcpsock.h"
#include "sbuffer.h"
#include "config.h"

#define ERROR_HANDLER(condition, msg) \
    do { if (condition) { fprintf(stderr, "Error: %s\n", msg); exit(EXIT_FAILURE); } } while (0)

typedef struct {
    tcpsock_t *client;
    sbuffer_t *buffer;
} thread_arg_t;

// Thread function to handle client communication
void *client_handler(void *arg) {
    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    tcpsock_t *client = thread_arg->client;
    sbuffer_t *buffer = thread_arg->buffer;
    free(thread_arg);

    sensor_data_t data;
    int bytes;

    while (1) {
        // Receive sensor ID
        bytes = sizeof(data.id);
        if (tcp_receive(client, (void *)&data.id, &bytes) != TCP_NO_ERROR || bytes == 0) {
            printf("Client disconnected or error\n");
            break;
        }

        // Receive sensor value
        bytes = sizeof(data.value);
        if (tcp_receive(client, (void *)&data.value, &bytes) != TCP_NO_ERROR || bytes == 0) {
            printf("Client disconnected or error\n");
            break;
        }

        // Receive timestamp
        bytes = sizeof(data.ts);
        if (tcp_receive(client, (void *)&data.ts, &bytes) != TCP_NO_ERROR || bytes == 0) {
            printf("Client disconnected or error\n");
            break;
        }

        // Log received data
        printf("Received data: ID = %(" PRIu16 "), Value = %f, Timestamp = %ld\n",
        data.id, data.value, (long)data.ts);


        // Write data to the shared buffer
        if (sbuffer_insert(buffer, &data) != SBUFFER_SUCCESS) {
            fprintf(stderr, "Error inserting data into buffer\n");
            break;
        }
    }

    tcp_close(&client);
    return NULL;
}

// Main connection manager function
void connmgr_listen(int port, int max_clients, sbuffer_t *buffer) {
    tcpsock_t *server, *client;
    pthread_t thread_id;
    int active_clients = 0;

    ERROR_HANDLER(tcp_passive_open(&server, port) != TCP_NO_ERROR, "Failed to open TCP socket");

    printf("Connection manager listening on port %d\n", port);

    while (active_clients < max_clients) {
        if (tcp_wait_for_connection(server, &client) == TCP_NO_ERROR) {
            printf("Client connected\n");
            thread_arg_t *arg = malloc(sizeof(thread_arg_t));
            arg->client = client;
            arg->buffer = buffer;

            if (pthread_create(&thread_id, NULL, client_handler, arg) != 0) {
                fprintf(stderr, "Error creating client thread\n");
                tcp_close(&client);
                free(arg);
                continue;
            }
            pthread_detach(thread_id); // No need to join threads
            active_clients++;
        } else {
            fprintf(stderr, "Error accepting client connection\n");
        }
    }

    tcp_close(&server);
    printf("Connection manager shutting down\n");
}
