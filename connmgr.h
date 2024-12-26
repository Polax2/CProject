//
// Created by polan on 20/12/2024.
//

#ifndef CONNMGR_H
#define CONNMGR_H

#include <pthread.h>
#include "sbuffer.h"
#include "lib/tcpsock.h"

struct connmgr_args {
    int args[3];  // Port, max_clients, timeout
    sbuffer_t *buffer;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    int log_fd;
    pthread_mutex_t *pipe_mutex;
    tcpsock_t *client;
};

void connmgr_listen(struct connmgr_args *args);
void *handle_client(void *arg);

#endif /* CONNMGR_H */
