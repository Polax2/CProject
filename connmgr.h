//
// Created by polan on 20/12/2024.
//
#ifndef CONNMGR_H_
#define CONNMGR_H_

#include <stdint.h>
#include "sbuffer.h"

// Starts the connection manager, listening for incoming connections.
// Accepts a port number and maximum number of clients.
// Writes sensor data into the shared buffer.
void connmgr_listen(int port, int max_clients, sbuffer_t *buffer);

#endif // CONNMGR_H_

