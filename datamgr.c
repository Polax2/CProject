#include "datamgr.h"
#include "config.h"
#include "sbuffer.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

extern int log_pipe_fd[2];
extern pthread_mutex_t pipe_mutex;

void *datamgr_process(void *arg) {
    sbuffer_t *buffer = (sbuffer_t *)arg;
    sensor_data_t *data;

    while (1) {
        data = sbuffer_read(buffer);  // Do not remove immediately
        if (data == NULL) break;

        char msg[256];
        snprintf(msg, sizeof(msg), "Processing Data (ID: %d, Value: %.2f) \n", data->id, data->value);
        log_to_logger(msg);

        pthread_mutex_lock(&buffer->mutex);
        buffer->head->end_flag = true;  // Data Manager finished processing
        pthread_mutex_unlock(&buffer->mutex);

        sleep(1);
    }

    pthread_exit(NULL);
}


