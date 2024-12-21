//
// Created by polan on 20/12/2024.
//
#ifndef CONNMGR_H_
#define CONNMGR_H_

#include "sbuffer.h"

// Listen for incoming sensor node connections
void connmgr_listen(int port, int max_clients, sbuffer_t *buffer);

// Close all connections and clean up
void connmgr_free();

#endif // CONNMGR_H_
