#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "datamgr.h"
#include "sbuffer.h"
#include "config.h"
#include <inttypes.h>
#include <unistd.h>

#define RUNNING_AVG_COUNT 5
#define TEMP_THRESHOLD_HIGH 25.0
#define TEMP_THRESHOLD_LOW 18.0

typedef struct {
    uint16_t sensor_id;
    uint16_t room_id;
    double running_avg;
    double last_values[RUNNING_AVG_COUNT];
    int value_index;
    int value_count;
} sensor_room_t;

static sensor_room_t *sensor_rooms = NULL;
static int sensor_room_count = 0;
static pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;

// Maps sensor ID to room ID
static int find_room_by_sensor(uint16_t sensor_id) {
    for (int i = 0; i < sensor_room_count; i++) {
        if (sensor_rooms[i].sensor_id == sensor_id) {
            return i;
        }
    }
    return -1;
}

// Initializes the Data Manager
int datamgr_init(const char *map_file) {
    FILE *fp = fopen(map_file, "r");
    if (!fp) {
        perror("Failed to open room-sensor map file");
        return -1;
    }

    int count = 0;
    uint16_t room_id, sensor_id;
    while (fscanf(fp, "%" SCNu16 " %" SCNu16, &sensor_id, &room_id) == 2) {
        count++;
    }
    fseek(fp, 0, SEEK_SET);

    sensor_rooms = malloc(sizeof(sensor_room_t) * count);
    if (!sensor_rooms) {
        perror("Failed to allocate memory for sensors");
        fclose(fp);
        return -1;
    }

    sensor_room_count = 0;
    while (fscanf(fp, "%" SCNu16 " %" SCNu16, &sensor_id, &room_id) == 2) {
        sensor_rooms[sensor_room_count].sensor_id = sensor_id;
        sensor_rooms[sensor_room_count].room_id = room_id;
        sensor_rooms[sensor_room_count].running_avg = 0.0;
        memset(sensor_rooms[sensor_room_count].last_values, 0, sizeof(double) * RUNNING_AVG_COUNT);
        sensor_rooms[sensor_room_count].value_index = 0;
        sensor_rooms[sensor_room_count].value_count = 0;
        sensor_room_count++;
    }
    fclose(fp);
    return 0;
}

// Frees Data Manager resources
void datamgr_free() {
    free(sensor_rooms);
    sensor_rooms = NULL;
    sensor_room_count = 0;
}

// Write sensor data to CSV
void write_to_csv(sensor_room_t *sensor) {
    FILE *csv_file = fopen("data.csv", "a");
    if (csv_file) {
        // Add header if the file is empty
        fseek(csv_file, 0, SEEK_END);
        if (ftell(csv_file) == 0) {
            fprintf(csv_file, "SensorID,Value,Timestamp\n");
        }

        fprintf(csv_file, "%hu,%.2f,%ld\n",
                sensor->sensor_id,
                sensor->running_avg,
                time(NULL));
        fclose(csv_file);
    } else {
        perror("Failed to open data.csv");
    }
}

// Process incoming sensor data
static void process_sensor_data(const sensor_data_t *data) {
    pthread_mutex_lock(&data_mutex);

    int idx = find_room_by_sensor(data->id);
    if (idx == -1) {
        printf("Sensor ID %d not found in mapping.\n", data->id);
        pthread_mutex_unlock(&data_mutex);
        return;
    }

    sensor_room_t *sensor = &sensor_rooms[idx];

    // Update running average
    sensor->last_values[sensor->value_index] = data->value;
    sensor->value_index = (sensor->value_index + 1) % RUNNING_AVG_COUNT;
    if (sensor->value_count < RUNNING_AVG_COUNT) {
        sensor->value_count++;
    }

    double sum = 0.0;
    for (int i = 0; i < sensor->value_count; i++) {
        sum += sensor->last_values[i];
    }
    sensor->running_avg = sum / sensor->value_count;

    // Threshold check
    if (sensor->running_avg > TEMP_THRESHOLD_HIGH) {
        printf("Room %d is too hot (Avg: %.2f)\n", sensor->room_id, sensor->running_avg);
    } else if (sensor->running_avg < TEMP_THRESHOLD_LOW) {
        printf("Room %d is too cold (Avg: %.2f)\n", sensor->room_id, sensor->running_avg);
    }

    printf("Writing to CSV: SensorID = %hu, Avg = %.2f\n",
            sensor->sensor_id, sensor->running_avg);

    write_to_csv(sensor);  // Call CSV writer
    pthread_mutex_unlock(&data_mutex);
}

// Data Manager process loop
void *datamgr_process(void *arg) {
    sbuffer_t *buffer = (sbuffer_t *)arg;
    sensor_data_t data;

    while (1) {
        if (sbuffer_remove(buffer, &data) == SBUFFER_SUCCESS) {
            printf("Processing SensorID: %hu, Value: %.2f\n",
                    data.id, data.value);
            process_sensor_data(&data);
        }
        return NULL;
    }

}
