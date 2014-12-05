#ifndef PPPROTOCOL_H
#define PPPROTOCOL_H

#include <unistd.h>   	//for pipe
#include <pthread.h>  	//thread/mutex
#include <select.h>   	//select
#include <sys/types.h> 	//select const
#include <stdio.h> 		//perror
#include <"message.h">

#define NUM_PROTOCOLS = 8

class ppProtocol
{
public:
	ppProtocol();
	~ppProtocol();
	registerSendProtocol(int protocolID, int pipeFD, pthread_mutex_t pipeMutex);
	registerRecvProtocol(int protocolID, int pipeFD, pthread_mutex_t pipeMutex);
	void registerHLP(ppProtocol hlp);
	void registerLLP(ppProtocol llp);
	void getRecv(int pipe[], pthread_mutex_t mutex[]);
	void getSend(int *pipe);




private:
	//pipe write is atomic and we're reading 
	//fixed message size --  no mutex
	// PROBABLY

	//receive from hlp (outbound msg)
	int sendPipe;
	// pthread_mutex_t sendMutex;

	//receive from llp (inbound msg)
	int recvPipe;
	// pthread_mutex_t recvMutex;

	//llp pipe (write outbound msg)
	int llpPipe;
	// pthread_mutex_t* llpMutex;

	//hlpPipe (write inbound msg)
	int hlpPipe[NUM_PROTOCOLS];
	// pthread_mutex_t* hlpMutex[NUM_PROTOCOLS];

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
	// fd_set sendFDset;
	// fd_set recvFDset;
	int protocolID;
	
};

#endif