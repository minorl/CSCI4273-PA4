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

class ppProtocol
{
	typedef ethHeader Header;
public:
	ppProtocol();
	~ppProtocol();
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
  	ppETH(int listenPort, const char* sendPort, const char* sendHost);
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
  //private:
	//int protocolID = 2;	
};
class ppTCP: public ppProtocol{
	typedef tcpHeader Header;
  //private:
	//int protocolID = 3;
};
class ppUDP: public ppProtocol{
	typedef udpHeader Header;
  //private:
	//int protocolID = 4;
};
class ppFTP: public ppProtocol{
	typedef ftpHeader Header;
  //private:
	//int protocolID = 5;
};
class ppTEL: public ppProtocol{
	typedef telHeader Header;
  //private:
	//int protocolID = 6;
};
class ppRDP: public ppProtocol{
	typedef rdpHeader Header;
  //private:
	//int protocolID = 7;
};
class ppDNS: public ppProtocol{
	typedef dnsHeader Header;
  //private:
	//int protocolID = 8;
};

class ppAPP : public ppProtocol{
	//needs no hlp- highest level
	//generates messages
	typedef ftpHeader Header;
	void* handleRecv(void); 
	static void *handleRecvHelper(void *instance) {
		return ((ppAPP*)instance)->handleRecv();
	}
	void* handleSend(void); 
	static void *handleSendHelper(void *instance) {
		return ((ppAPP*)instance)->handleSend();
	}
};

class ftpAPP : public ppAPP{
	typedef ftpHeader Header;
};

class dnsAPP : public ppAPP{
	typedef dnsHeader Header;
};

class telAPP : public ppAPP{
	typedef telHeader Header;
};

class rdpAPP : public ppAPP{
	typedef rdpHeader Header;
};
#endif