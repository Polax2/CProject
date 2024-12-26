#ifndef _SBUFFER_H_
#define _SBUFFER_H_

#include "config.h"

#define SBUFFER_SUCCESS 0
#define SBUFFER_FAILURE -1

typedef struct sbuffer sbuffer_t;

int sbuffer_init(sbuffer_t **buffer);
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data);
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data);
void sbuffer_free(sbuffer_t *buffer);


#endif  // SBUFFER_H
