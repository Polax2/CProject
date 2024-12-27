#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "connmgr.h"
#include "datamgr.h"
#include "sbuffer.h"
#include "sensor_db.h"

sbuffer_t *shared_buffer;
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_cond = PTHREAD_COND_INITIALIZER;

// Pipe and logger variables
int log_pipe_fd[2];  // 0 - Read, 1 - Write
pthread_mutex_t pipe_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t connmgr_tid, storagemgr_tid, datamgr_tid;

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

// Main process logic - Creates 3 threads
void main_process(int port, int max_clients) {
    if (sbuffer_init(&shared_buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize shared buffer\n");
        exit(EXIT_FAILURE);
    }

    // Start connection, data, and storage manager threads
    pthread_create(&connmgr_tid, NULL, (void *)connmgr_listen, shared_buffer);
    pthread_create(&storagemgr_tid, NULL, (void *)sensor_db_process, shared_buffer);
    pthread_create(&datamgr_tid, NULL, (void *)datamgr_process, shared_buffer);

    // Wait for connection and storage managers to finish
    pthread_join(connmgr_tid, NULL);
    pthread_join(storagemgr_tid, NULL);
    pthread_join(datamgr_tid, NULL);

    sbuffer_free(shared_buffer);
    pthread_mutex_destroy(&buffer_mutex);
    pthread_mutex_destroy(&pipe_mutex);
    pthread_cond_destroy(&buffer_cond);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <max_clients>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    int max_clients = atoi(argv[2]);

    // Create pipe for inter-process communication
    if (pipe(log_pipe_fd) < 0) {
        perror("Pipe creation failed");
        return EXIT_FAILURE;
    }

    int pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        // Child (logger process)
        close(log_pipe_fd[1]);  // Close write-end in child
        logger_process();
        close(log_pipe_fd[0]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent (main process)
        close(log_pipe_fd[0]);  // Close read-end in parent
        main_process(port, max_clients);
        close(log_pipe_fd[1]);
        wait(NULL);  // Wait for child to terminate
    }

    return EXIT_SUCCESS;
}
