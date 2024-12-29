#define _POSIX_C_SOURCE 200809L

#include "datamgr.h"
#include "sbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include "config.h"


extern int log_pipe_fd[2];
extern pthread_mutex_t pipe_mutex;

void *datamgr_process(void *buffer) {
    // sbuffer_t *shared_buffer = (sbuffer_t *)buffer;
    // sensor_data_t data;

    while (1) {
        char msg[256];
        snprintf(msg, 256, "Pozdrawiam z datamgr\n");
        log_to_logger(msg);
        sleep(20);
        // if (sbuffer_remove(shared_buffer, &data) == SBUFFER_SUCCESS) {
        //     log_event("Processing data - ID: %hu, Value: %.2f", data.id, data.value);
        //     if (data.value < SET_MIN_TEMP) {
        //         log_event("ALERT: Sensor ID %hu reports LOW temperature: %.2f", data.id, data.value);
        //     } else if (data.value > SET_MAX_TEMP) {
        //         log_event("ALERT: Sensor ID %hu reports HIGH temperature: %.2f", data.id, data.value);
        //     }
        // } else {
        //     struct timespec ts = {0, 500000000};
        //     nanosleep(&ts, NULL);
        // }
        sleep(1);
    }
    pthread_exit(NULL);
}
