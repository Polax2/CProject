#ifndef CONNMGR_H
#define CONNMGR_H

#include <pthread.h>
#include "sbuffer.h"

typedef struct {
    sbuffer_t *buffer;
    pthread_mutex_t *mutex;
} connmgr_args_t;

//void connmgr_listen(struct connmgr_args *args);
void connmgr_listen(connmgr_args_t *args);
#endif // CONNMGR_H

