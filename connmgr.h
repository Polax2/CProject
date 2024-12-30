#ifndef CONNMGR_H
#define CONNMGR_H

#include <pthread.h>
#include "sbuffer.h"


// Connection manager argument structure
// struct connmgr_args {
//     int port;
//     int max_clients;
//     sbuffer_t *buffer;
//     pthread_mutex_t *mutex;
//     pthread_cond_t *cond;
//     int log_fd;
//     pthread_mutex_t *pipe_mutex;
//     tcpsock_t *client;
// };


struct connmgr_args {
    sbuffer_t *buffer;
    pthread_mutex_t *mutex;
};

//void connmgr_listen(struct connmgr_args *args);
void connmgr_listen(struct connmgr_args *args);
#endif // CONNMGR_H

