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
        data = sbuffer_read(buffer);  // Read from buffer
        if (data == NULL) {
            if (buffer->end_flag) break;
            continue;
        }

        char msg[256];
        snprintf(msg, sizeof(msg), "Processing Data (ID: %d, Value: %.2f) \n", data->id, data->value);
        log_to_logger(msg);

        // Check if the temperature is too low or too high
        if (data->value < SET_MIN_TEMP) {
            snprintf(msg, sizeof(msg), "Sensor node %d reports it’s too cold (avg temp = %.2f)\n",
                     data->id, data->value);
            log_to_logger(msg);
        } else if (data->value > SET_MAX_TEMP) {
            snprintf(msg, sizeof(msg), "Sensor node %d reports it’s too hot (avg temp = %.2f)\n",
                     data->id, data->value);
            log_to_logger(msg);
        }

        pthread_mutex_lock(&buffer->mutex);
        buffer->head->end_flag = true;  // Mark node for removal by Storage Manager
        pthread_mutex_unlock(&buffer->mutex);

        sleep(1);  // Simulate processing delay
    }

    pthread_exit(NULL);
}
