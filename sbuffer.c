//
// Created by polan on 20/12/2024.
//

#include "sbuffer.h"
#include <stdlib.h>
#include <string.h>

struct sbuffer {
    sensor_data_t data;
    struct sbuffer *next;
};

int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->next = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_t *new_node = malloc(sizeof(sbuffer_t));
    if (new_node == NULL) return SBUFFER_FAILURE;

    memcpy(&new_node->data, data, sizeof(sensor_data_t));
    new_node->next = buffer->next;
    buffer->next = new_node;
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    if (buffer->next == NULL) return SBUFFER_FAILURE;
    sbuffer_t *temp = buffer->next;
    memcpy(data, &temp->data, sizeof(sensor_data_t));
    buffer->next = temp->next;
    free(temp);
    return SBUFFER_SUCCESS;
}

void sbuffer_free(sbuffer_t *buffer) {
    sbuffer_t *current = buffer;
    while (current != NULL) {
        sbuffer_t *temp = current;
        current = current->next;
        free(temp);
    }
}
