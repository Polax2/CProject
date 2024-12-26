//
// Created by polan on 20/12/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include "connmgr.h"
#include "datamgr.h"
#include "sensor_db.h"
#include "sbuffer.h"
#include "config.h"

// Shared buffer
sbuffer_t *shared_buffer;

// Pipe and mutex for logging
int log_pipe_fd;
pthread_mutex_t pipe_mutex = PTHREAD_MUTEX_INITIALIZER;

void *log_process(void *arg) {
    int pipe_fd = *((int *)arg);
    FILE *log_file = fopen("gateway.log", "w");
    if (!log_file) {
        perror("Failed to open gateway.log");
        return NULL;
    }

    char buffer[256];
    while (1) {
        ssize_t bytes_read = read(pipe_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            if (strcmp(buffer, "TERMINATE") == 0) {
                fprintf(log_file, "Log process received termination signal.\n");
                fflush(log_file);
                break;
            }
            fprintf(log_file, "%s\n", buffer);
            fflush(log_file);
        }
    }
    fclose(log_file);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> <max_clients>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    int max_clients = atoi(argv[2]);

    // Initialize shared buffer
    if (sbuffer_init(&shared_buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "ERROR: Shared buffer initialization failed\n");
        return EXIT_FAILURE;
    }

    // Create pipe for logging
    int fd[2];
    if (pipe(fd) < 0) {
        perror("ERROR: Pipe creation failed");
        return EXIT_FAILURE;
    }
    log_pipe_fd = fd[1];

    // Fork to create logger process
    pid_t log_pid = fork();
    if (log_pid < 0) {
        perror("Fork failed");
        return EXIT_FAILURE;
    }

    if (log_pid == 0) {
        // Child process - Logger
        close(fd[1]);  // Close write end
        log_process(&fd[0]);  // Log to gateway.log
        close(fd[0]);
        return EXIT_SUCCESS;
    }

    // Parent process
    close(fd[0]);  // Close read end

    pthread_t connmgr_tid, datamgr_tid, storagemgr_tid;

    // Initialize connection manager arguments
    struct connmgr_args conn_args = {
        .args = {port, max_clients, 10},
        .buffer = shared_buffer,
        .mutex = &pipe_mutex,
        .cond = NULL,
        .log_fd = log_pipe_fd,
        .pipe_mutex = &pipe_mutex,
        .client = NULL
    };

    // Start threads for connection, data management, and storage
    pthread_create(&connmgr_tid, NULL, (void *(*)(void *))connmgr_listen, &conn_args);
    pthread_create(&datamgr_tid, NULL, (void *(*)(void *))datamgr_process, shared_buffer);
    pthread_create(&storagemgr_tid, NULL, (void *(*)(void *))sensor_db_process, shared_buffer);

    pthread_join(connmgr_tid, NULL);
    pthread_join(datamgr_tid, NULL);
    pthread_join(storagemgr_tid, NULL);

    // Cleanup
    sbuffer_free(shared_buffer);
    pthread_mutex_destroy(&pipe_mutex);

    // Send termination signal to logger process
    write(fd[1], "TERMINATE", strlen("TERMINATE") + 1);
    close(fd[1]);
    waitpid(log_pid, NULL, 0);

    return EXIT_SUCCESS;
}
