#include "sensor_db.h"
#include "config.h"
#include "sbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

extern int log_pipe_fd[2];
extern pthread_mutex_t pipe_mutex;

void *sensor_db_process(void *arg) {
    sbuffer_t *buffer = (sbuffer_t *)arg;
    sensor_data_t data;
    FILE *csv_file = fopen("data.csv", "w");

    if (!csv_file) {
        log_to_logger("Unable to open CSV file.\n");
        pthread_exit(NULL);
    }

    log_to_logger("CSV file opened successfully.\n");

    fprintf(csv_file, "SensorID,Value,Timestamp\n");
    log_to_logger("CSV header written.\n");

    while (1) {
        pthread_mutex_lock(&buffer->mutex);

        if (buffer->head == NULL) {
            log_to_logger("Buffer is empty. Waiting for data...\n");
        }

        if (sbuffer_remove(buffer, &data) == SBUFFER_SUCCESS) {
            fprintf(csv_file, "%d,%.2f,%ld\n", data.id, data.value, data.ts);
            fflush(csv_file);

            char msg[256];
            snprintf(msg, sizeof(msg), "Data saved (ID: %d, Value: %.2f, Timestamp: %ld)", data.id, data.value, data.ts);
            log_to_logger(msg);
        } else {
            log_to_logger("No data to write to CSV. Exiting process.\n");
            pthread_mutex_unlock(&buffer->mutex);
            break;
        }

        pthread_mutex_unlock(&buffer->mutex);
    }

    log_to_logger("Closing CSV file.\n");
    fclose(csv_file);
    pthread_exit(NULL);
}
