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
static FILE *data_file = NULL;

void *sensor_db_process(void *buffer) {
    sbuffer_t *shared_buffer = (sbuffer_t *)buffer;
    sensor_data_t data;

    while (1) {
        if (sbuffer_remove(shared_buffer, &data) == SBUFFER_SUCCESS) {
            if (!data_file) {
                data_file = fopen("data.csv", "w");
                if (!data_file) {
                    log_event("Failed to open data.csv");
                    exit(EXIT_FAILURE);
                }
                fprintf(data_file, "sensor_id,value,timestamp\n");
                fflush(data_file);
                log_event("Created new data.csv");
            }
            fprintf(data_file, "%hu,%.2f,%ld\n", data.id, data.value, (long)data.ts);
            fflush(data_file);
            log_event("Stored data - ID: %hu, Value: %.2f", data.id, data.value);
        } else {
            struct timespec ts = {0, 500000000};  // 500 ms delay
            nanosleep(&ts, NULL);
        }
    }
    pthread_exit(NULL);
}
