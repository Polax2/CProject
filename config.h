#ifndef SET_MIN_TEMP
#define SET_MIN_TEMP = 10
#endif

#ifndef SET_MAX_TEMP
#define SET_MAX_TEMP = 30
#endif

#ifndef _CONFIG_H_
#define _CONFIG_H_


#include <stdint.h>
#include <pthread.h>
#include <time.h>

typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;

extern int pipe_fd[2];
extern pthread_mutex_t pipe_mutex;

typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;
} sensor_data_t;

void log_to_logger(const char *msg);

#endif /* _CONFIG_H_ */
