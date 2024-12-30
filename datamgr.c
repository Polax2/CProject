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
    sensor_data_t data;

    while (1) {
        pthread_mutex_lock(&buffer->mutex);
        if (sbuffer_remove(buffer, &data) == SBUFFER_SUCCESS) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Processing Data (ID: %d, Value: %.2f)", data.id, data.value);

            if (data.value < SET_MIN_TEMP) {
                snprintf(msg, sizeof(msg), "emperature too low (ID: %d, Value: %.2f)", data.id, data.value);
                log_to_logger(msg);
            } else if (data.value > SET_MAX_TEMP) {
                snprintf(msg, sizeof(msg), "Temperature too high (ID: %d, Value: %.2f)", data.id, data.value);
                log_to_logger(msg);
            }
        } else {
            pthread_mutex_unlock(&buffer->mutex);
            break;
        }
        pthread_mutex_unlock(&buffer->mutex);
    }
    pthread_exit(NULL);
}
