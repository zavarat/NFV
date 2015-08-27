#---------------Final Target----------------#

all:	ran mme hss sgw pgw sink

#-------------------------------------------#


#-----------------Variables-----------------#

G++ = g++ -std=c++0x -std=gnu++0x

RAN_H	= utils.h packet.h client.h ue.h enodeb.h
RAN_CPP = utils.cpp packet.cpp client.cpp ue.cpp enodeb.cpp

MME_H = utils.h packet.h thread_pool.h server.h client.h mme.h
MME_CPP = utils.cpp packet.cpp thread_pool.cpp server.cpp client.cpp mme.cpp

HSS_H = utils.h packet.h thread_pool.h server.h db_mysql.h
HSS_CPP = utils.cpp packet.cpp thread_pool.cpp server.cpp db_mysql.cpp

SGW_H = utils.h packet.h thread_pool.h server.h client.h sgwc.h sgwu.h
SGW_CPP = utils.cpp packet.cpp thread_pool.cpp server.cpp client.cpp sgwc.cpp sgwu.cpp

PGW_H = utils.h packet.h thread_pool.h server.h client.h raw_socket.h pgwc.h pgwc_monitor.h pgwu.h
PGW_CPP = utils.cpp packet.cpp thread_pool.cpp server.cpp client.cpp raw_socket.cpp pgwc.cpp pgwc_monitor.cpp pgwu.cpp

SINK_H = utils.h packet.h thread_pool.h server.h client.h sink_monitor.h
SINK_CPP = utils.cpp packet.cpp thread_pool.cpp server.cpp client.cpp sink_monitor.cpp

#-------------------------------------------#


#---------Prerequisites and Recipes---------#

RAN_P = ran.h ran.cpp $(RAN_H) $(RAN_CPP)
RAN_R = $(G++) ran.cpp -w -o ran $(RAN_CPP) -lpthread 


MME_P = mme_server.h mme_server.cpp $(MME_H) $(MME_CPP)
MME_R = $(G++) mme_server.cpp -w -o mme $(MME_CPP) -lpthread 


HSS_P = hss.h hss.cpp $(HSS_H) $(HSS_CPP)
HSS_R = $(G++) hss.cpp -w -o hss $(HSS_CPP) `mysql_config --cflags --libs`


SGW_P = sgw_server.h sgw_server.cpp $(SGW_H) $(SGW_CPP)
SGW_R = $(G++) sgw_server.cpp -w -o sgw $(SGW_CPP) -lpthread 


PGW_P = pgw_server.h pgw_server.cpp $(PGW_H) $(PGW_CPP)
PGW_R = $(G++) pgw_server.cpp -w -o pgw $(PGW_CPP) -lpthread 


SINK_P = sink_server.h sink_server.cpp $(SINK_H) $(SINK_CPP)
SINK_R = $(G++) sink_server.cpp -w -o sink $(SINK_CPP) -lpthread

#-------------------------------------------#


#---------------Make Statements-------------#

ran:	$(RAN_P)
	$(RAN_R)

mme:	$(MME_P)
	$(MME_R)
	
hss:	$(HSS_P)
	$(HSS_R)

sgw:	$(SGW_P)
	$(SGW_R)

pgw:	$(PGW_P)
	$(PGW_R)

sink:	$(SINK_P)
	$(SINK_R)

clean:
	rm -f ran mme hss sgw pgw *~

#-------------------------------------------#

#----------Special Commands-----------------#

# make -k (To keep going even after encountering errors in making a former target)

#-------------------------------------------#