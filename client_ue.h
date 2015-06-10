#include <iostream>
#include <string>
#include <netdb.h>
#include <cstdlib>
#include <pthread.h>
using namespace std;

struct UE_data{
	int key;
	uint64_t IMSI;
	uint64_t MSISDN;
};

struct connection_data{
	int type;
	int sock_fd;
	pthread_t tid;
};

class UE{
	public:
		UE_data ue;
		connection_data connect;
		unsigned char packet_attach[9];
		unsigned char packet_response[17];
		
		UE(int);
		~UE();		
		void packetize();		
		void handleErrors(void);				
		char* read_ue(unsigned char*, int);
		int write_ue(unsigned char*, int);		
};
