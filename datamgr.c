#define _POSIX_C_SOURCE 200809L  // Ensure POSIX compatibility for nanosleep

#include "datamgr.h"
#include "sbuffer.h"
#include "config.h"
#include "connmgr.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>  // For nanosleep

extern sbuffer_t *shared_buffer;  // Link to shared buffer from main.c

// Data manager process function
void *datamgr_process(void *buffer) {
    sensor_data_t data;
    struct timespec ts = {0, 10000};  // 0 sec + 20000 nanoseconds (20 microseconds)

    while (1) {
        if (sbuffer_remove(shared_buffer, &data) == SBUFFER_SUCCESS) {
            log_event("Processing data: ID: %hu Value: %.2f Timestamp: %ld",
                      data.id, data.value, (long)data.ts);

            if (data.value < SET_MIN_TEMP) {
                log_event("ALERT: Sensor %hu reports LOW temperature: %.2f",
                          data.id, data.value);
            } else if (data.value > SET_MAX_TEMP) {
                log_event("ALERT: Sensor %hu reports HIGH temperature: %.2f",
                          data.id, data.value);
            }
        } else {
            // Sleep to avoid busy waiting (replaces usleep)
            nanosleep(&ts, NULL);
        }
    }
    return NULL;
}
