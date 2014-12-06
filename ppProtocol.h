#ifndef PPPROTOCOL_H
#define PPPROTOCOL_H

#include <unistd.h>   	//for pipe
#include <pthread.h>  	//thread/mutex
#include <sys/types.h> 	//
#include <stdio.h> 		//perror
#include <stdlib.h> 	//exit
#include "headers.h"
#include "message.h"

#define NUM_PROTOCOLS 8

class ppProtocol
{
public:
	ppProtocol();
	~ppProtocol();
	void registerHLP(ppProtocol hlp);
	void registerLLP(ppProtocol llp);
	void getRecv(int pipe[]);
	void getSend(int *pipe);

	//receive from hlp (outbound msg)
	int sendPipe[2];
	//receive from llp (inbound msg)
	int recvPipe[2];
	//llp pipe (write outbound msg)
	int llpPipe;
	//hlpPipe (write inbound msg)
	int hlpPipe[NUM_PROTOCOLS];


private:
	//pipe write is atomic and we're reading 
	//fixed message size --  no mutex
	// PROBABLY




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

#endif