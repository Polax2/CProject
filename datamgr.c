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
        data = sbuffer_read(buffer);  // Read without removing
        if (data == NULL) {
            printf("Data Manager: No more data to process. Terminating...\n");
            break;
        }

        char msg[256];
        snprintf(msg, sizeof(msg), "Processing Data (ID: %d, Value: %.2f)", data->id, data->value);
        log_to_logger(msg);

        if (data->value < SET_MIN_TEMP) {
            snprintf(msg, sizeof(msg), "Temperature too low (ID: %d, Value: %.2f)", data->id, data->value);
            log_to_logger(msg);
        } else if (data->value > SET_MAX_TEMP) {
            snprintf(msg, sizeof(msg), "Temperature too high (ID: %d, Value: %.2f)", data->id, data->value);
            log_to_logger(msg);
        }

        sleep(1);  // Simulate processing delay
    }

    pthread_exit(NULL);
}
