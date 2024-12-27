#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "lib/tcpsock.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <sensor_id> <value_count> <server_ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int sensor_id = atoi(argv[1]);
    int value_count = atoi(argv[2]);
    char *server_ip = argv[3];
    int port = atoi(argv[4]);

    tcpsock_t *client;
    if (tcp_active_open(&client, port, server_ip) != TCP_NO_ERROR) {
        fprintf(stderr, "Failed to connect to server at %s:%d\n", server_ip, port);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < value_count; i++) {
        sensor_data_t data = {
            .id = sensor_id,
            .value = rand() % 100,
            .ts = time(NULL)
        };
        int bytes = sizeof(data);
        tcp_send(client, (void *)&data, &bytes);
        sleep(1);
    }

    tcp_close(&client);
    return EXIT_SUCCESS;
}
