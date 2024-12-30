#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "connmgr.h"
#include "datamgr.h"
#include "sbuffer.h"
#include "sensor_db.h"
#include "config.h"

#define TIMEOUT 5  // Timeout in seconds

sbuffer_t *shared_buffer;
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_cond = PTHREAD_COND_INITIALIZER;

// Pipe and logger variables
int log_pipe_fd[2];  // 0 - Read, 1 - Write
pthread_mutex_t pipe_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t connmgr_tid, storagemgr_tid, datamgr_tid;

void log_to_logger(const char *msg) {
    pthread_mutex_lock(&pipe_mutex);
    ssize_t bytes_written = write(log_pipe_fd[1], msg, strlen(msg));
    if (bytes_written < 0) {
        perror("Log pipe write failed");
    }
    pthread_mutex_unlock(&pipe_mutex);
}


// Logger function to handle log writing
void logger_process() {
    FILE *log_file = fopen("gateway.log", "w");
    if (!log_file) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    char *line;
    int log_id = 0;
    while (1) {
        ssize_t bytes_read = read(log_pipe_fd[0], buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';  // Null-terminate the string

            line = strtok(buffer, "\n");
            while (line != NULL) {
                time_t now = time(NULL);
                struct tm *timeinfo = localtime(&now);

                char timestamp[20];
                strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

                fprintf(log_file, "%6d %s %s\n", log_id++, timestamp, line);
                fflush(log_file);

                line = strtok(NULL, "\n");  // Get next line
            }
        } else {
            sleep(2);
        }
    }
}

// Main process logic - Creates 3 threads
void main_process(int port, int max_clients) {
    shared_buffer = sbuffer_init();
    if (shared_buffer == NULL) {
        fprintf(stderr, "Failed to initialize shared buffer\n");
        exit(EXIT_FAILURE);
    }

    // Start connection, data, and storage manager threads
    if (pthread_create(&connmgr_tid, NULL, (void *)connmgr_listen, shared_buffer) != 0) {
        perror("Failed to create connection manager thread");
    }
    if (pthread_create(&storagemgr_tid, NULL, (void *)sensor_db_process, shared_buffer) != 0) {
        perror("Failed to create storage manager thread");
    }
    if (pthread_create(&datamgr_tid, NULL, (void *)datamgr_process, shared_buffer) != 0) {
        perror("Failed to create data manager thread");
    }

    pthread_join(connmgr_tid, NULL);
    pthread_join(storagemgr_tid, NULL);
    pthread_join(datamgr_tid, NULL);

    sbuffer_free(shared_buffer);  // Pass directly
    shared_buffer = NULL;

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
