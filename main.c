#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include "connmgr.h"
#include "sbuffer.h"
#include "sensor_db.h"

sbuffer_t *shared_buffer;
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_cond = PTHREAD_COND_INITIALIZER;

// Pipe and logger variables
int log_pipe_fd[2];  // 0 - Read, 1 - Write
pthread_mutex_t pipe_mutex = PTHREAD_MUTEX_INITIALIZER;

// Logger function to handle log writing
void logger_process() {
    FILE *log_file = fopen("gateway.log", "a");
    if (!log_file) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    while (1) {
        ssize_t bytes_read = read(log_pipe_fd[0], buffer, sizeof(buffer));
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            fprintf(log_file, "%s\n", buffer);
            fflush(log_file);
        }
    }
}

// Main sensor gateway process
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <max_clients>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    int max_clients = atoi(argv[2]);

    if (pipe(log_pipe_fd) < 0) {
        perror("Pipe creation failed");
        return EXIT_FAILURE;
    }

    int pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        close(log_pipe_fd[1]);  // Close write-end in child
        logger_process();
        exit(EXIT_SUCCESS);
    }

    close(log_pipe_fd[0]);  // Close read-end in parent

    if (sbuffer_init(&shared_buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize shared buffer\n");
        return EXIT_FAILURE;
    }

    pthread_t connmgr_tid, storagemgr_tid;

    struct connmgr_args conn_args = {
        .port = port,
        .max_clients = max_clients,
        .buffer = shared_buffer,
        .mutex = &buffer_mutex,
        .cond = &buffer_cond,
        .log_fd = log_pipe_fd[1],  // Pass write-end to connection manager
        .pipe_mutex = &pipe_mutex
    };

    pthread_create(&connmgr_tid, NULL, (void *(*)(void *))connmgr_listen, &conn_args);
    pthread_create(&storagemgr_tid, NULL, sensor_db_process, shared_buffer);

    pthread_join(connmgr_tid, NULL);
    pthread_join(storagemgr_tid, NULL);

    sbuffer_free(shared_buffer);
    pthread_mutex_destroy(&buffer_mutex);
    pthread_mutex_destroy(&pipe_mutex);
    pthread_cond_destroy(&buffer_cond);

    close(log_pipe_fd[1]);  // Close write-end after finishing

    return EXIT_SUCCESS;
}
