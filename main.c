//
// Created by polan on 20/12/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include "connmgr.h"
#include "sbuffer.h"

int main() {
    sbuffer_t *buffer = NULL;

    // Initialize the shared buffer
    if (sbuffer_init(&buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize shared buffer\n");
        return EXIT_FAILURE;
    }

    // Start the connection manager
    // Port: 12345, Max Clients: 5
    connmgr_listen(12345, 5, buffer);

    // Free the shared buffer after the connection manager is done
    if (sbuffer_free(&buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to free shared buffer\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
