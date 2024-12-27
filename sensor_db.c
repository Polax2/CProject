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
#include "connmgr.h"


extern pthread_mutex_t pipe_mutex;  // Reference the mutex from main.c
extern void log_event(const char *format, ...);  // Reference log_event from connmgr.c

static FILE *data_file = NULL;

void sensor_db_init(const char *filename) {
    data_file = fopen(filename, "w");
    if (!data_file) {
        log_event("Failed to open %s for writing", filename);
        perror("File open error");
        exit(EXIT_FAILURE);
    }
    fprintf(data_file, "sensor_id,value,timestamp\n");
    fflush(data_file);
    log_event("Sensor DB initialized, logging to %s", filename);
}

void *sensor_db_process(void *buffer) {
    sbuffer_t *shared_buffer = (sbuffer_t *)buffer;
    sensor_data_t data;

    while (sbuffer_remove(shared_buffer, &data) == SBUFFER_SUCCESS) {
        pthread_mutex_lock(&pipe_mutex);  // Mutex for writing to data.csv
        fprintf(data_file, "%hu,%.2f,%ld\n", data.id, data.value, (long)data.ts);
        fflush(data_file);
        pthread_mutex_unlock(&pipe_mutex);

        log_event("Stored data - ID: %hu Value: %.2f Timestamp: %ld",
                  data.id, data.value, (long)data.ts);
    }
    log_event("Sensor DB process completed");
    return NULL;
}

void sensor_db_cleanup() {
    if (data_file) {
        fclose(data_file);
        log_event("Closed data.csv");
    }
}
