#ifndef RAN_H
#define RAN_H

#include "utils.h"
#include "packet.h"
#include "client.h"
#include "ue.h"
#include "enodeb.h"

extern EnodeB g_enodeb;
extern int g_total_connections;
extern double g_req_duration;
extern time_t g_start_time;

void setup_tun();
void* monitor_traffic(void*);
void* generate_traffic(void*);
void attach(UE&, Client&);
void send_traffic(UE&);
void detach(UE&, Client&);
void startup_ran(char**, vector<int>&, vector<pthread_t>&);

#endif //RAN_H