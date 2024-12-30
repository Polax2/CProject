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
    sensor_data_t *data;
    FILE *csv_file = fopen("data.csv", "w");

    if (!csv_file) {
        log_to_logger("Unable to open CSV file.\n");
        pthread_exit(NULL);
    }

    log_to_logger("CSV file opened successfully.\n");
    fprintf(csv_file, "SensorID,Value,Timestamp\n");
    fflush(csv_file);

    while (1) {
        data = sbuffer_read(buffer);

        if (data == NULL) {
            log_to_logger("Storage Manager: Buffer empty. Waiting for data...\n");
            break;
        }

        fprintf(csv_file, "%d,%.2f,%ld\n", data->id, data->value, data->ts);
        fflush(csv_file);

        char msg[256];
        snprintf(msg, sizeof(msg), "Data saved (ID: %d, Value: %.2f, Timestamp: %ld) \n",
                 data->id, data->value, data->ts);
        log_to_logger(msg);

        // Mark for removal and delete the node
        sbuffer_mark_for_removal(buffer);
        sbuffer_remove_marked(buffer);
    }

    log_to_logger("Closing CSV file.\n");
    fclose(csv_file);
    pthread_exit(NULL);
}