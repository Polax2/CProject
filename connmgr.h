#ifndef CONNMGR_H
#define CONNMGR_H

#include <pthread.h>
#include "sbuffer.h"
#include "lib/tcpsock.h"

// Connection manager argument structure
struct connmgr_args {
    int port;
    int max_clients;
    sbuffer_t *buffer;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    int log_fd;
    pthread_mutex_t *pipe_mutex;
    tcpsock_t *client;
};

// Function to listen for connections
void connmgr_listen(struct connmgr_args *args);

// Declare log_event as external (used for logging)
extern void log_event(const char *format, ...);

#endif // CONNMGR_H
