#include "sgwc.h"

unordered_map<int, int> g_bearer_table;
unordered_map<uint16_t, TunCdata> g_tun_ctable;

TunCdata::TunCdata(){


}

TunCdata::~TunCdata(){


}

SGWc::SGWc(){

	type = 1;
	ue_ip = allocate_str_mem(INET_ADDRSTRLEN);
	reply = allocate_str_mem(BUFFER_SIZE);
}

void SGWc::create_session_req_from_mme(Server &sgw_server){

	copy_data(sgw_server.pkt);
	set_ue_num();
	set_bearer_id();
	set_cteid();
}

void SGWc::copy_data(Packet &arg){

	pkt.clear_data();
	pkt.fill_data(0, arg.data_len, arg.data);
}

void SGWc::set_ue_num(){

	memcpy(&ue_num, pkt.data + sizeof(int), sizeof(int));
}

void SGWc::set_bearer_id(){

	memcpy(&bearer_id, pkt.data + 2*sizeof(int), sizeof(int));
	fill_bearer_table();
}

void SGWc::fill_bearer_table(){

	g_bearer_table[ue_num] = bearer_id;
}

void SGWc::set_cteid(){

	cteid = generate_cteid(ue_num);
}

uint16_t SGWc::generate_cteid(int &ue_number){

	return ue_number;
}

void SGWc::create_session_req_to_pgw(uint16_t &uteid){

	connect_with_pgw();
	to_pgw.pkt.clear_data();
	to_pgw.pkt.fill_data(0, sizeof(int), ue_num);
	to_pgw.pkt.fill_data(sizeof(int), sizeof(int), bearer_id);
	to_pgw.pkt.fill_data(2*sizeof(int), sizeof(uint16_t), cteid);
	to_pgw.pkt.fill_data(2*sizeof(int) + sizeof(uint16_t), sizeof(uint16_t), uteid);
	to_pgw.pkt.make_data_packet();
	to_pgw.write_data();
}

void SGWc::connect_with_pgw(){

	to_pgw.bind_client();
	to_pgw.fill_server_details(g_pgw_port, g_pgw_addr);
	to_pgw.connect_with_server(ue_num);
	handshake_with_pgw();
}

void SGWc::handshake_with_pgw(){

	to_pgw.pkt.clear_data();
	to_pgw.pkt.fill_data(0, sizeof(int), type);
	to_pgw.pkt.make_data_packet();
	to_pgw.write_data();
}

void SGWc::create_session_res_from_pgw(uint16_t &pgw_uteid){

	to_pgw.read_data();
	to_pgw.pkt.rem_gtpc_hdr();
	set_tun_cdata();
	memcpy(&pgw_uteid, to_pgw.pkt.data + sizeof(uint16_t), sizeof(uint16_t));
	set_ue_ip();	
}

void SGWc::set_tun_cdata(){

	memcpy(&tun_cdata.mme_cteid, pkt.data + 3*sizeof(int), sizeof(uint16_t));
	memcpy(&tun_cdata.pgw_cteid, to_pgw.pkt.data, sizeof(uint16_t));
	tun_cdata.pgw_port = g_pgw_port;
	tun_cdata.pgw_addr.assign(g_pgw_addr);
}

void SGWc::set_ue_ip(){

	memcpy(ue_ip, to_pgw.pkt.data + 2*sizeof(uint16_t), INET_ADDRSTRLEN);		
}

void SGWc::create_session_res_to_mme(Server &sgw_server){

	strcpy(reply, "OK");
	sgw_server.pkt.clear_data();
	sgw_server.pkt.fill_data(0, sizeof(uint16_t), cteid);
	sgw_server.pkt.fill_data(sizeof(uint16_t), strlen(reply), reply);
	sgw_server.pkt.fill_gtpc_hdr(tun_cdata.mme_cteid);
	sgw_server.pkt.add_gtpc_hdr();
	sgw_server.pkt.make_data_packet();
	sgw_server.write_data();
}

void SGWc::modify_session_req_from_mme(Server &sgw_server, uint16_t &enodeb_uteid){

	sgw_server.read_data();
	sgw_server.pkt.rem_gtpc_hdr();
	copy_data(sgw_server.pkt);
	memcpy(&enodeb_uteid, pkt.data, sizeof(uint16_t));
}

void SGWc::modify_session_res_to_mme(Server &sgw_server, uint16_t &sgw_uteid){

	strcpy(reply, "OK");
	sgw_server.pkt.clear_data();
	sgw_server.pkt.fill_data(0, sizeof(uint16_t), sgw_uteid);
	sgw_server.pkt.fill_data(sizeof(uint16_t), INET_ADDRSTRLEN, ue_ip);
	sgw_server.pkt.fill_data(sizeof(uint16_t) + INET_ADDRSTRLEN, strlen(reply), reply);
	sgw_server.pkt.fill_gtpc_hdr(tun_cdata.mme_cteid);
	sgw_server.pkt.add_gtpc_hdr();
	sgw_server.pkt.make_data_packet();
	sgw_server.write_data();
}

void SGWc::fill_pgw_addr(int &pgw_port, string &pgw_addr){

	pgw_port = tun_cdata.pgw_port;
	pgw_addr = tun_cdata.pgw_addr;
}

void SGWc::fill_tun_ctable(){

	g_tun_ctable[cteid] = tun_cdata;
}

void SGWc::delete_session_req_from_mme(Server &sgw_server){
	int type;

	sgw_server.read_data();
	sgw_server.pkt.rem_gtpc_hdr();
	memcpy(&type, sgw_server.pkt.data, sizeof(int));
	copy_pkts(sgw_server.pkt, to_pgw.pkt);
	if(type == 3){
		cout<<"Detach request for UE - "<<ue_num<<" has been received at SGW"<<endl;
	}
}

void SGWc::copy_pkts(Packet &from_pkt, Packet &to_pkt){

	to_pkt.clear_data();
	to_pkt.fill_data(0, from_pkt.data_len, from_pkt.data);
}

void SGWc::delete_session_req_to_pgw(){

	to_pgw.pkt.fill_gtpc_hdr(tun_cdata.pgw_cteid);
	to_pgw.pkt.add_gtpc_hdr();
	to_pgw.pkt.make_data_packet();
	to_pgw.write_data();
	erase_tun_ctable();
}

void SGWc::erase_bearer_table(){

	g_bearer_table.erase(ue_num);
}

void SGWc::erase_tun_ctable(){

	g_tun_ctable.erase(cteid);
}

void SGWc::delete_session_res_from_pgw(){
	string res;

	to_pgw.read_data();
	to_pgw.pkt.rem_gtpc_hdr();
	memcpy(reply, to_pgw.pkt.data, to_pgw.pkt.data_len);
	res.assign(reply);
	if(res == "OK"){
		cout<<"PGW has successfully deallocated resources for UE - "<<ue_num<<endl;
	}
}

void SGWc::delete_session_res_to_mme(Server &sgw_server){
	string res = "OK";

	sgw_server.pkt.clear_data();
	sgw_server.pkt.fill_data(0, res.size(), res);
	sgw_server.pkt.fill_gtpc_hdr(tun_cdata.mme_cteid);
	sgw_server.pkt.add_gtpc_hdr();
	sgw_server.pkt.make_data_packet();
	sgw_server.write_data();
}


SGWc::~SGWc(){

	free(ue_ip);
	free(reply);
}