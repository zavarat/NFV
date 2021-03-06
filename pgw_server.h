#ifndef PGW_SERVER_H
#define PGW_SERVER_H

#include "utils.h"
#include "packet.h"
#include "thread_pool.h"
#include "server.h"
#include "client.h"
#include "raw_socket.h"
#include "pgwc.h"
#include "pgwc_monitor.h"
#include "pgwu.h"

void setup_tun();
void* monitor_traffic(void*);
void* process_traffic(void*);
void handle_cdata(Server&);
void handle_udata(Server&);

#endif //PGW_SERVER_H
