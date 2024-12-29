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


extern int log_pipe_fd;
extern pthread_mutex_t pipe_mutex;


void *sensor_db_process(void *buffer) {
    // sbuffer_t *shared_buffer = (sbuffer_t *)buffer;
    // sensor_data_t data;

    while (1) {
        sleep(1);
    }
}