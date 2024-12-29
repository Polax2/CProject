#define _POSIX_C_SOURCE 200809L
#include "sensor_db.h"
#include "sbuffer.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdarg.h>
#include <unistd.h>


extern int log_pipe_fd[2];
extern pthread_mutex_t pipe_mutex;


void *sensor_db_process(void *buffer) {
    // sbuffer_t *shared_buffer = (sbuffer_t *)buffer;
    // sensor_data_t data;

    while (1) {
        char msg[256];
        snprintf(msg, 256, "Pozdrawiam z sensor_db\n");
        log_to_logger(msg);
        sleep(20);
        sleep(1);
    }

}

