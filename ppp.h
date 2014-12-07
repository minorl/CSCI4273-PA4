#ifndef PPPROTOCOL_H
#define PPPROTOCOL_H

#include <unistd.h>   	//for pipe
#include <pthread.h>  	//thread/mutex
#include <sys/types.h> 	//
#include <stdio.h> 		//perror
#include <stdlib.h> 	//exit
#include <netinet/in.h> //sockets
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include "headers.h"
#include "message.h"

#define NUM_PROTOCOLS 8

//for timing
extern int count;
extern int numApps;
extern pthread_mutex_t count_mutex;
extern pthread_cond_t count_threshold_cv;

class ppProtocol
{
	typedef ethHeader Header;
public:
	ppProtocol(int id = 0);
	~ppProtocol();
	void start();
	void registerHLP(ppProtocol hlp);
	void registerLLP(ppProtocol llp);
	void getRecv(int pipe[]);
	void getSend(int *pipe);
	void setID(int id);


	//for debug
	//receive from hlp (outbound msg)
	int sendPipe[2];
	//receive from llp (inbound msg)
	int recvPipe[2];
	//llp pipe (write outbound msg)
	int llpPipe;
	//hlpPipe (write inbound msg)
	int hlpPipe[NUM_PROTOCOLS];

	//pipe write is atomic and we're reading 
	//fixed message size --  no mutex
	// PROBABLY

	// //receive from hlp (outbound msg)
	// int sendPipe[2];
	// //receive from llp (inbound msg)
	// int recvPipe[2];
	// //llp pipe (write outbound msg)
	// int llpPipe;
	// //hlpPipe (write inbound msg)
	// int hlpPipe[NUM_PROTOCOLS];

	void* handleRecv(void); 
	static void *handleRecvHelper(void *instance) {
		return ((ppProtocol*)instance)->handleRecv();
	}
	void* handleSend(void); 
	static void *handleSendHelper(void *instance) {
		return ((ppProtocol*)instance)->handleSend();
	}
	pthread_t sendThread;
	pthread_t recvThread;
	int protocolID;

};

class ppETH: public ppProtocol{
	typedef ethHeader Header;
  public:
  	// ppETH();
  	ppETH(int id, int listenPort, const char* sendPort, const char* sendHost);
  	~ppETH();
  	int getUDPSend(const char* host, const char* portnum);
  	int bindUDPListen(int port);
  	void* handleRecv(void); 
	static void *handleRecvHelper(void *instance) {
		return ((ppETH*)instance)->handleRecv();
	}
	void* handleSend(void); 
	static void *handleSendHelper(void *instance) {
		return ((ppETH*)instance)->handleSend();
	}
	void sendUDP(int fd, char* message, int length);
  // private:
  	//host needs to set up and connect sockets
	int recvReadSocket;
	int recvSendSocket;
};
class ppIP: public ppProtocol{
	typedef ipHeader Header;
  public:
  	ppIP(int id) : ppProtocol(id){};
};
class ppTCP: public ppProtocol{
	typedef tcpHeader Header;
  public:
  	ppTCP(int id) : ppProtocol(id){};	
};
class ppUDP: public ppProtocol{
	typedef udpHeader Header;
  public:
  	ppUDP(int id) : ppProtocol(id){};
};
class ppFTP: public ppProtocol{
	typedef ftpHeader Header;
  public:
  	ppFTP(int id) : ppProtocol(id){};
};
class ppTEL: public ppProtocol{
	typedef telHeader Header;
  public:
  	ppTEL(int id) : ppProtocol(id){};
};
class ppRDP: public ppProtocol{
	typedef rdpHeader Header;
  public:
  	ppRDP(int id) : ppProtocol(id){};
};
class ppDNS: public ppProtocol{
	typedef dnsHeader Header;
  public:
  	ppDNS(int id) : ppProtocol(id){};
};

class ppAPP : public ppProtocol{
	//needs no hlp- highest level
	//generates messages when started
	typedef ftpHeader Header;
  public:
	ppAPP(int id, bool timer);
	void* handleRecv(void); 
	static void *handleRecvHelper(void *instance) {
		return ((ppAPP*)instance)->handleRecv();
	}
	void* handleSend(void); 
	static void *handleSendHelper(void *instance) {
		return ((ppAPP*)instance)->handleSend();
	}
	void start();
	void startListen();
	//signal global condition variable, set up timing
	bool timing;
};

class ftpAPP : public ppAPP{
	typedef ftpHeader Header;
  public:
  	ftpAPP(int id, bool timing) : ppAPP(id, timing){};
};

class dnsAPP : public ppAPP{
	typedef dnsHeader Header;
  public:
  	dnsAPP(int id, bool timing) : ppAPP(id, timing){};	
};

class telAPP : public ppAPP{
	typedef telHeader Header;
  public:
  	telAPP(int id, bool timing) : ppAPP(id, timing){};	
};

class rdpAPP : public ppAPP{
	typedef rdpHeader Header;
  public:
  	rdpAPP(int id, bool timing) : ppAPP(id, timing){};	
};
#endif