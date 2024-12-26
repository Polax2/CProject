//
// Created by polan on 20/12/2024.
//

#include "datamgr.h"
#include "lib/dplist.h"
#include "sensor_db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#define RUN_AVG_LENGTH 5
#ifndef SET_MAX_TEMP
#define SET_MAX_TEMP 25.0
#endif

#ifndef SET_MIN_TEMP
#define SET_MIN_TEMP 10.0
#endif

// Mutex for thread synchronization
static pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;

// Global list to store sensor-room mapping
static dplist_t *sensor_rooms = NULL;
static int log_pipe_fd;

// Sensor node structure
typedef struct {
    uint16_t sensor_id;
    uint16_t room_id;
    double running_avg;
    double temp_values[RUN_AVG_LENGTH];
    int temp_count;
} sensor_node_t;

// Callback functions for dplist
void *element_copy(void *element) {
    sensor_node_t *new_node = malloc(sizeof(sensor_node_t));
    memcpy(new_node, element, sizeof(sensor_node_t));
    return new_node;
}

void element_free(void **element) {
    free(*element);
    *element = NULL;
}

int element_compare(void *x, void *y) {
    return ((sensor_node_t *)x)->sensor_id - ((sensor_node_t *)y)->sensor_id;
}

// Initialize the data manager
void datamgr_init(const char *map_file, int pipe) {
    FILE *fp = fopen(map_file, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening map file\n");
        exit(EXIT_FAILURE);
    }

    sensor_rooms = dpl_create(element_copy, element_free, element_compare);
    log_pipe_fd = pipe;

    uint16_t room_id, sensor_id;
    while (fscanf(fp, "%hu %hu", &room_id, &sensor_id) == 2) {
        sensor_node_t new_sensor = {
            .sensor_id = sensor_id,
            .room_id = room_id,
            .running_avg = 0.0,
            .temp_count = 0
        };
        memset(new_sensor.temp_values, 0, sizeof(new_sensor.temp_values));
        dpl_insert_at_index(sensor_rooms, &new_sensor, dpl_size(sensor_rooms), true);
    }
    fclose(fp);
}

// Process data from the shared buffer
void *datamgr_process(void *buffer) {
    sbuffer_t *sbuffer = (sbuffer_t *)buffer;
    sensor_data_t data;

    while (sbuffer_remove(sbuffer, &data) == SBUFFER_SUCCESS) {
        pthread_mutex_lock(&data_mutex);

        for (int i = 0; i < dpl_size(sensor_rooms); i++) {
            sensor_node_t *sensor = dpl_get_element_at_index(sensor_rooms, i);
            if (sensor->sensor_id == data.id) {
                sensor->temp_values[sensor->temp_count % RUN_AVG_LENGTH] = data.value;
                sensor->temp_count++;

                double sum = 0.0;
                int count = (sensor->temp_count < RUN_AVG_LENGTH) ? sensor->temp_count : RUN_AVG_LENGTH;
                for (int j = 0; j < count; j++) {
                    sum += sensor->temp_values[j];
                }
                sensor->running_avg = sum / count;

                // Log temperature alerts
                char log_msg[256];
                if (sensor->running_avg > SET_MAX_TEMP) {
                    snprintf(log_msg, sizeof(log_msg), "Room %hu too hot! Avg: %.2f\n",
                             sensor->room_id, sensor->running_avg);
                    write(log_pipe_fd, log_msg, strlen(log_msg));
                } else if (sensor->running_avg < SET_MIN_TEMP) {
                    snprintf(log_msg, sizeof(log_msg), "Room %hu too cold! Avg: %.2f\n",
                             sensor->room_id, sensor->running_avg);
                    write(log_pipe_fd, log_msg, strlen(log_msg));
                }
            }
        }
        pthread_mutex_unlock(&data_mutex);
    }
    return NULL;
}

// Free all resources
void datamgr_free() {
    pthread_mutex_lock(&data_mutex);
    dpl_free(&sensor_rooms, true);
    pthread_mutex_unlock(&data_mutex);
}
