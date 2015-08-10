#include "sgw.h"

void TunData::TunData(){
	
	mme_addr = allocate_str_mem(INET_ADDRSTRLEN);
	pgw_addr = allocate_str_mem(INET_ADDRSTRLEN);
	enodeb_addr = allocate_str_mem(INET_ADDRSTRLEN);
}

void TunData::~TunData(){
	
	free(mme_addr);
	free(pgw_addr);
	free(enodeb_addr);
}

void SGW::SGW(){
	
	ip_addr = allocate_str_mem(INET_ADDRSTRLEN);
}

void SGW::~SGW(){

	free(ip_addr);
}

void SGW::set_sgw_cteid(int ue_num){
	
	sgw_cteid =  ue_num;
}

void SGW::set_sgw_uteid(int ue_num){
	
	sgw_uteid =  ue_num;
}

void* process_traffic(void *arg){
	int type;
	ClientDetails entity = *(ClientDetails*)arg;
	Server sgw;
	Tunnel tun;

	tun.set_sgw_cteid(entity.num);
	tun.set_sgw_uteid(entity.num);
	sgw.fill_server_details(g_freeport, g_sgw1_addr);
	sgw.bind_server();
	sgw.client_sock_addr = entity.client_sock_addr;
	sgw.client_num = entity.num;
	sgw.connect_with_client();
	sgw.read_data();
	memcpy(&type, sgw.pkt.data, sizeof(int));
	if(type == 1){
		handle_cdata(sgw, tun, entity.num);
	}
	else if(type == 2)
		handle_udata(sgw, tun, entity.num);
}

void handle_cdata(Server &sgw, Tunnel &tun, int &ue_num){
	int user_num;
	int bearer_id;
	int type = 1;
	char *ue_ip_addr = allocate_str_mem(INET_ADDRSTRLEN);
	char *reply = allocate_str_mem(BUFFER_SIZE);
	Client to_pgw;
	
	to_pgw.bind_client();
	to_pgw.fill_server_details(g_pgw_port, g_pgw_addr);
	to_pgw.connect_with_server(ue_num);
	memcpy(&user_num, sgw.pkt.data + sizeof(int), sizeof(int));
	memcpy(&bearer_id, sgw.pkt.data + 2*sizeof(int), sizeof(int));
	memcpy(&tun.mme_cteid, sgw.pkt.data + 3*sizeof(int), sizeof(uint16_t));
	set_bearer_id(user_num, bearer_id);
	to_pgw.pkt.clear_data();
	to_pgw.pkt.fill_data(0, sizeof(int), type);
	to_pgw.pkt.fill_data(sizeof(int), sizeof(int), user_num);
	to_pgw.pkt.fill_data(2*sizeof(int), sizeof(int), bearer_id);
	to_pgw.pkt.fill_data(3*sizeof(int), sizeof(uint16_t), tun.sgw_cteid);
	to_pgw.pkt.fill_data(3*sizeof(int) + sizeof(uint16_t), sizeof(uint16_t), tun.sgw_uteid);
	to_pgw.pkt.make_data_packet();
	to_pgw.write_data();
	to_pgw.read_data();
	to_pgw.pkt.rem_gtpc_hdr();
	memcpy(&tun.pgw_cteid, to_pgw.pkt.data, sizeof(uint16_t));
	memcpy(&tun.pgw_uteid, to_pgw.pkt.data + sizeof(uint16_t), sizeof(uint16_t));
	memcpy(ue_ip_addr, to_pgw.pkt.data + 2*sizeof(uint16_t), INET_ADDRSTRLEN);
	strcpy(reply, "OK");
	sgw.pkt.clear_data();
	sgw.pkt.fill_data(0, sizeof(uint16_t), tun.sgw_cteid);
	sgw.pkt.fill_data(sizeof(uint16_t), strlen(reply), reply);
	sgw.pkt.fill_gtpc_hdr(tun.mme_cteid);
	sgw.pkt.add_gtpc_hdr();
	sgw.pkt.make_data_packet();
	sgw.write_data();
	sgw.read_data();
	sgw.pkt.rem_gtpc_hdr();
	memcpy(&tun.enodeb_uteid, sgw.pkt.data, sizeof(uint16_t));
	sgw.pkt.clear_data();
	sgw.pkt.fill_data(0, sizeof(uint16_t), tun.sgw_uteid);
	sgw.pkt.fill_data(sizeof(uint16_t), INET_ADDRSTRLEN, ue_ip_addr);
	sgw.pkt.fill_data(sizeof(uint16_t) + INET_ADDRSTRLEN, strlen(reply), reply);
	sgw.pkt.add_gtpc_hdr();
	sgw.pkt.make_data_packet();
	sgw.write_data();
	cout<<"Tunnel is formed from eNodeB to PGW via SGW for UE - "<<ue_num<<endl;
	free(ue_ip_addr);
	free(reply);
}

void handle_udata(Server &sgw, Tunnel &tun, int &ue_num){
	Client to_pgw;
	int type = 2;
	
	sgw.read_data();
	to_pgw.bind_client();
	to_pgw.fill_server_details(g_pgw_port, g_pgw_addr);
	to_pgw.connect_with_server(ue_num);
	to_pgw.pkt.clear_data();
	to_pgw.pkt.fill_data(0, sizeof(int), type);
	to_pgw.pkt.make_data_packet();
	to_pgw.write_data();
	sgw.pkt.rem_gtpu_hdr();

	char *addr = allocate_str_mem(INET_ADDRSTRLEN); 
	int port;
	memcpy(&sgw.pkt.ip_hdr, sgw.pkt.data, IP_LEN * sizeof(uint8_t));
	memcpy(&sgw.pkt.udp_hdr, sgw.pkt.data + IP_LEN * sizeof(uint8_t), UDP_LEN * sizeof(uint8_t));
	port = ntohs(sgw.pkt.udp_hdr.source);
	inet_ntop(AF_INET, &(sgw.pkt.ip_hdr.ip_src), addr, INET_ADDRSTRLEN);
	cout<<"Port is "<<port<<endl;
	cout<<"Address is "<<addr<<endl;


	sgw.pkt.fill_gtpu_hdr(tun.pgw_uteid);
	sgw.pkt.add_gtpu_hdr();
	sgw.pkt.make_data_packet();
	to_pgw.pkt.clear_data();
	to_pgw.pkt.fill_data(0, sgw.pkt.packet_len, sgw.pkt.packet);
	to_pgw.pkt.make_data_packet();	
	to_pgw.write_data();
}

void set_bearer_id(int ue_num, int bearer_id){
	
	g_bearer_table[ue_num] = bearer_id;
}

int main(){
	Server sgw;
	
	sgw.fill_server_details(g_sgw1_port, g_sgw1_addr);
	sgw.bind_server();
	sgw.listen_accept(process_traffic);
	return 0;
}