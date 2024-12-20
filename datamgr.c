//
// Created by polan on 20/12/2024.
//

#include "datamgr.h"
#include "dplist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SET_MAX_TEMP
#define SET_MAX_TEMP 25.0
#endif

#ifndef SET_MIN_TEMP
#define SET_MIN_TEMP 10.0
#endif

typedef struct {
    uint16_t sensor_id;
    uint16_t room_id;
    double running_avg;
    double temp_values[RUN_AVG_LENGTH];
    int temp_count;
} sensor_node_t;

static dplist_t *sensor_list = NULL;

// Callback functions for the doubly linked list
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
void datamgr_init(const char *map_file) {
    FILE *fp = fopen(map_file, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open room-sensor map file\n");
        exit(EXIT_FAILURE);
    }

    sensor_list = dpl_create(element_copy, element_free, element_compare);

    uint16_t room_id, sensor_id;
    while (fscanf(fp, "%hu %hu", &room_id, &sensor_id) == 2) {
        sensor_node_t new_sensor = {
            .sensor_id = sensor_id,
            .room_id = room_id,
            .running_avg = 0.0,
            .temp_count = 0
        };
        memset(new_sensor.temp_values, 0, sizeof(new_sensor.temp_values));
        sensor_list = dpl_insert_at_index(sensor_list, &new_sensor, dpl_size(sensor_list), true);
    }

    fclose(fp);
}

// Process data from the shared buffer
void datamgr_process(sbuffer_t *buffer) {
    sensor_data_t data;

    while (sbuffer_remove(buffer, &data) == SBUFFER_SUCCESS) {
        sensor_node_t *sensor = NULL;
        for (int i = 0; i < dpl_size(sensor_list); i++) {
            sensor_node_t *node = dpl_get_element_at_index(sensor_list, i);
            if (node->sensor_id == data.id) {
                sensor = node;
                break;
            }
        }

        if (sensor == NULL) {
            fprintf(stderr, "Warning: Sensor ID %hu not found in map file\n", data.id);
            continue;
        }

        // Update running average
        sensor->temp_values[sensor->temp_count % RUN_AVG_LENGTH] = data.value;
        sensor->temp_count++;

        double sum = 0.0;
        int count = (sensor->temp_count < RUN_AVG_LENGTH) ? sensor->temp_count : RUN_AVG_LENGTH;
        for (int i = 0; i < count; i++) {
            sum += sensor->temp_values[i];
        }
        sensor->running_avg = sum / count;

        // Check temperature thresholds
        if (sensor->running_avg > SET_MAX_TEMP) {
            fprintf(stderr, "Sensor %hu in room %hu: Too hot (avg = %.2f)\n",
                    sensor->sensor_id, sensor->room_id, sensor->running_avg);
        } else if (sensor->running_avg < SET_MIN_TEMP) {
            fprintf(stderr, "Sensor %hu in room %hu: Too cold (avg = %.2f)\n",
                    sensor->sensor_id, sensor->room_id, sensor->running_avg);
        }
    }
}

// Free resources used by the data manager
void datamgr_free() {
    dpl_free(&sensor_list, true);
}

