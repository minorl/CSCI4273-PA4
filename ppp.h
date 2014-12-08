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

	template <class Header> class ppProtocol
	{
	public:
		// typedef ethHeader Header;

		ppProtocol(int id = 0);
		~ppProtocol();
		void start();
		template <class T>
		void registerHLP(ppProtocol<T> hlp);
		template <class T>
		void registerLLP(ppProtocol<T> llp);
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
			return ((ppProtocol<Header>*)instance)->handleRecv();
		}
		void* handleSend(void); 
		static void *handleSendHelper(void *instance) {
			return ((ppProtocol<Header>*)instance)->handleSend();
		}
		pthread_t sendThread;
		pthread_t recvThread;
		int protocolID;

	};

	class ppETH: public ppProtocol<ethHeader>{
	  public:
	  	// ppETH();
		// typedef ethHeader Header;
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
	class ppIP: public ppProtocol<ipHeader>{
	  public:
		// typedef ipHeader Header;
	  	ppIP(int id) : ppProtocol(id){};
	};
	class ppTCP: public ppProtocol<tcpHeader>{
	  public:
		// typedef tcpHeader Header;
	  	ppTCP(int id) : ppProtocol(id){};	
	};
	class ppUDP: public ppProtocol<udpHeader>{
	  public:
		// typedef udpHeader Header;
	  	ppUDP(int id) : ppProtocol(id){};
	};
	class ppFTP: public ppProtocol<ftpHeader>{
	  public:
		// typedef ftpHeader Header;
	  	ppFTP(int id) : ppProtocol(id){};
	};
	class ppTEL: public ppProtocol<telHeader>{
	  public:
		// typedef telHeader Header;
	  	ppTEL(int id) : ppProtocol(id){};
	};
	class ppRDP: public ppProtocol<rdpHeader>{
	  public:
		// typedef rdpHeader Header;
	  	ppRDP(int id) : ppProtocol(id){};
	};
	class ppDNS: public ppProtocol<dnsHeader>{
	  public:
		// typedef dnsHeader Header;
	  	ppDNS(int id) : ppProtocol(id){};
	};

	template <class Header>
	class ppAPP : public ppProtocol<Header>{
		//needs no hlp- highest level
		//generates messages when started
	  public:
		// typedef ftpHeader Header;

		ppAPP(int id, bool timer);
		void* handleRecv(void); 
		static void *handleRecvHelper(void *instance) {
			return ((ppAPP*)instance)->handleRecv();
		}
		void* handleSend(void); 
		static void *handleSendHelper(void *instance) {
			return ((ppAPP*)instance)->handleSend();
		}
		void startApplication();
		void startListen();
		//signal global condition variable, set up timing
		bool timing;
	};

	class ftpAPP : public ppAPP<ftpHeader>{
	  public:
		// typedef ftpHeader Header;
	  	ftpAPP(int id, bool timing) : ppAPP(id, timing){};
	};

	class dnsAPP : public ppAPP<dnsHeader>{
	  public:
		// typedef dnsHeader Header;
	  	dnsAPP(int id, bool timing) : ppAPP(id, timing){};	
	};

	class telAPP : public ppAPP<telHeader>{
	  public:
		// typedef telHeader Header;
	  	telAPP(int id, bool timing) : ppAPP(id, timing){};	
	};

	class rdpAPP : public ppAPP<rdpHeader>{
	  public:
		// typedef rdpHeader Header;
	  	rdpAPP(int id, bool timing) : ppAPP(id, timing){};	
	};



	template<class Header> 
	ppProtocol<Header>::ppProtocol(int id){
		protocolID = id;
		if(pipe(sendPipe)<0){
			perror("pipe");
			exit(1);
		}
		if(pipe(recvPipe)<0){
			perror("pipe");
			exit(1);
		}

	}
	template <class Header> 
	void ppProtocol<Header>::start(){
		//turns out i have no idea how c++ inheritance works
		int rc;
		rc = pthread_create(&sendThread,NULL, &handleSendHelper,this);
		if(rc){
		    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
			exit(1);	
		}
		rc = pthread_create(&recvThread,NULL, &handleRecvHelper,this);
		if(rc){
		    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
			exit(1);	
		}
	}
	template <class Header> 
	ppProtocol<Header>::~ppProtocol(){
		//leak baby leak
	}

	template <class Header>
	template <class T>
	void ppProtocol<Header>::registerHLP(ppProtocol<T> hlp){
		//this level of abstraction is unncessary
		// but it's so confusing the naming conventions
		//registers a higher level protocol
		// hlp.getRecv(hlpPipe, hlpMutex);
		hlp.getRecv(hlpPipe);
	}
	template <class Header> 
	template <class T>
	void ppProtocol<Header>::registerLLP(ppProtocol<T> llp){
		//registers a lower level protocol
		llp.getSend(&llpPipe);
	}
	template <class Header> 
	void ppProtocol<Header>::getRecv(int pipe[]){
		//passes own information to a llp trying to register
		// so send recv values
		//give WRITE end of recv pipe
		pipe[protocolID-1] = recvPipe[1];
		// mutex[protocolID-1] = &recvMutex;
	}
	template <class Header> 
	void ppProtocol<Header>::getSend(int *pipe){
		//passes own information to a hlp trying to register
		// so send values
		//give WRITE end of send pipe
		*pipe = sendPipe[1];
	}
	template <class Header> 
	void ppProtocol<Header>::setID(int id){
		protocolID = id;
	}
	template <class Header> 
	void* ppProtocol<Header>::handleSend(){
		// reads send pipe
		// which receives from hlp
		// handles outbound messages
		// send ot llp
		int nbytes;
		//expect int + msg ptr
		int packetSize = sizeof(int)+sizeof(char*);
		char buff[packetSize];
		int ptr_size, int_size;
		ptr_size = sizeof(char*);
		int_size = sizeof(int);
		int hlpID;
		Message* msgPtr;
		Header* head;
		while(true){
			//block here until something is written
			//write is ATOMIC so we don't need mutices
			nbytes = read(sendPipe[0], buff, packetSize);
			if(nbytes<0){
				perror("read");
				exit(1);
			}
			else if(nbytes < packetSize){
				fprintf(stderr, "Bytes read less than expected, big trouble.\n");
				fprintf(stderr, "Protoid %d, recvd %d, expected %d\n", protocolID, nbytes, packetSize);
			}
			//process received info
			memcpy(&hlpID, &buff, int_size);
			//this doesn't work=== why?
			// msgPtr = (Message*)(buff+sizeof(int));
			memcpy(&msgPtr, &buff[int_size], ptr_size);

			//create new header
			head = new Header;
			memset(head, 0, sizeof(*head));
			head->hlp = hlpID;
			head->len = msgPtr->msgLen();
			//add header to message

			msgPtr->msgAddHdr((char*)head, sizeof(*head));
			//send message down pipe
			   //include this id (hlp)
			memcpy(&buff, &protocolID, int_size);
			// *((int*)buff) = protocolID;
			//it should already be this
			// *(Message*)(buff+sizeof(int)) = msgPtr;

			//write message to llp
			// fprintf(stderr, "in Proto %d to llp:\n", protocolID);

			if(write(llpPipe, buff, packetSize)<0){
				fprintf(stderr, "in Proto %d to llp:\n", protocolID);
				perror("write");
				exit(1);
			}
		}
	}

	template <class Header> 
	void* ppProtocol<Header>::handleRecv(){
		//reads recv pipe, which receives from llp
		//process inc message, strip header and send up

		int nbytes;
		//expect msg ptr
		int packetSize = sizeof(char*);
		char buff[packetSize];
		int hlpID;
		Message* msgPtr;
		Header* head;

		while(true){
			//block here until something is written
			//write is ATOMIC so we don't need mutices
			if((nbytes = read(recvPipe[0], buff, packetSize))<0){
				perror("read");
				exit(1);
			}
			else if(nbytes < packetSize){
				fprintf(stderr, "Bytes read less than expected, big trouble.\n");
			}
			//process received info
			memcpy(&msgPtr, &buff, packetSize);
			// msgPtr = (Message*)buff;
			// printf("Protoid %d headerlen %lu\n",protocolID, sizeof(Header));
			
			//remove own level protocol
			head = (Header*)msgPtr->msgStripHdr(sizeof(Header));
			hlpID = head->hlp;


			//send message up pipe
			   //to appropriate hlp

			//this should already be set
			// (Message*)(buff) = msgPtr;

			//write message to hlp
			//write message to hlp
			if(hlpID == 0 || hlpID == 1 || hlpID == -1){
				//0 not valid, 1 is ethernet, not "above" anyone
			 fprintf(stderr, "Incorrect hlp: in Proto %d to %d:\n", protocolID, hlpID);
			 continue;
			}

			// fprintf(stderr, "in Proto %d to hlp %d:\n", protocolID, hlpID );

			if(write(hlpPipe[hlpID-1], buff, packetSize) <0){
				fprintf(stderr, "in Proto %d to %d:\n", protocolID, hlpID );
				perror("write");
				exit(1);
			}
		}
	}

	ppETH::ppETH(int id, int listenPort, const char* sendPort, const char* sendHost) : ppProtocol(id){
		//do this in start
		int rc;
		recvSendSocket = getUDPSend(sendHost, sendPort);
		sleep(2);
		recvReadSocket = bindUDPListen(listenPort);
		rc = pthread_create(&sendThread,NULL, &handleSendHelper,this);
		if(rc){
		    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
			exit(1);	
		}
		rc = pthread_create(&recvThread,NULL, &handleRecvHelper,this);
		if(rc){
		    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
			exit(1);	
		}
		if(pipe(sendPipe)<0){
			perror("pipe");
			exit(1);
		}

		//recv is from socket
	}
	ppETH::~ppETH(){
		close(recvReadSocket);
		close(recvSendSocket);
		// printf("DESTRUCTED\n");
	}

	int ppETH::getUDPSend(const char *host, const char *portnum){
	    struct hostent  *phe;   /* pointer to host information entry    */
	    struct sockaddr_in sin; /* an Internet endpoint address         */
	    int     s;              /* socket descriptor                    */
	    memset(&sin, 0, sizeof(sin));
	    sin.sin_family = AF_INET;
	    /* Map port number (char string) to port number (int)*/
	    if ((sin.sin_port=htons((unsigned short)atoi(portnum))) == 0){
	        perror("connectsock htons");
	        exit(1);
	    }

	    /* Map host name to IP address, allowing for dotted decimal */
	    if ( (phe = gethostbyname(host)) )
	        memcpy(&sin.sin_addr, phe->h_addr_list[0], phe->h_length);
	    else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE ){
	        perror("connectsock inet_addr");
	        exit(1);
	    }
	    /* Allocate a socket */
	    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	    if (s < 0){
	        perror("socket");
	        exit(1);
	    }

	    /* Connect the socket */
	    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0){
	        perror("passivesock connect");
	        exit(1);
	    }
	    return s;
	}

	int ppETH::bindUDPListen(int port){
	    int socketFileDescriptor;
	    struct sockaddr_in serverAddress;

	    if((socketFileDescriptor = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
	        perror("udpSock");
	        exit(1);    
	    }

	    serverAddress.sin_family = AF_INET;
	    serverAddress.sin_port = htons(port);
	    serverAddress.sin_addr.s_addr = INADDR_ANY;
	    memset(&(serverAddress.sin_zero),0,8);

	    if(bind(socketFileDescriptor,(struct sockaddr *)&serverAddress, sizeof(struct sockaddr)) == -1){
	        perror("Bind failure");
	        exit(1);
	    }

	    return socketFileDescriptor;
	}

	void* ppETH::handleSend(){
		// reads send pipe
		// which receives from hlp
		// handles outbound messages
		// send ot llp
		int nbytes;
		//expect int + msg ptr
		int packetSize = sizeof(char*)+sizeof(int);
		char buff[packetSize];
		int ptr_size, int_size;
		ptr_size = sizeof(char*);
		int_size = sizeof(int);
		int hlpID;
		Message* msgPtr;
		ethHeader* head;
		char sendbuff[1024];
		int msgLen;
		while(true){
			//block here until something is written
			//write is ATOMIC so we don't need mutices
			nbytes = read(sendPipe[0], buff, packetSize);
			if(nbytes<0){
				perror("read");
				exit(1);
			}
			else if(nbytes < packetSize){
				fprintf(stderr, "ETHSEND: Bytes read less than expected, big trouble.\n");
			}
			//process received info
			memcpy(&hlpID, &buff, int_size);
			//this doesn't work=== why?
			// msgPtr = (Message*)(buff+sizeof(int));
			memcpy(&msgPtr, &buff[int_size], ptr_size);

			//create new header
			head = new ethHeader;
			memset(head, 0, sizeof(*head));
			head->hlp = hlpID;
			head->len = msgPtr->msgLen();
			//add header to message
			msgPtr->msgAddHdr((char*)head, sizeof(*head));
			memset(sendbuff,0,1024);
			msgLen = htonl(msgPtr->msgLen());
			memcpy(&sendbuff, &msgLen, int_size);
			msgPtr->msgFlat(&sendbuff[int_size]);

			//write message to "wire"
			// if(write(recvSendSocket, thing, packetSize)<0){
			// 	perror("write");
			// 	exit(1);
			// }
			sendUDP(recvSendSocket,sendbuff, 1024);

		}
	}

	void* ppETH::handleRecv(){
		//reads recv pipe, which receives from llp
		//process inc message, strip header and send up
		int nbytes;
		//expect 1024 buffer
		int packetSize = 1024;
		char buff[packetSize];
		int hlpID;
		int msgLen;
		Message* msgPtr;
		ethHeader* head;
		char* charptr;

		while(true){
			//block here until something is written
			//write is ATOMIC so we don't need mutices
			if((nbytes = read(recvReadSocket, buff, packetSize))<0){
				perror("read");
				exit(1);
			}
			else if(nbytes < packetSize){
				fprintf(stderr, "ETHRECV: Bytes read less than expected, big trouble.\n");
			}
			//process received info
			memcpy(&msgLen, &buff, sizeof(int));
			msgLen = ntohl(msgLen);
			// printf("eth recv msg len %d\n",msgLen);

			
			charptr = new char[msgLen];
			memset(charptr, 0, msgLen);
			memcpy(charptr, &buff[sizeof(int)], msgLen);

			msgPtr = new Message(charptr, msgLen);
			//remove own level protocol
			head = (ethHeader*)msgPtr->msgStripHdr(sizeof(ethHeader));
			// printf("Protoid %d headerlen %lu\n",protocolID, sizeof(ethHeader));
			hlpID = head->hlp;

			memset(buff,0,packetSize);		
			memcpy(&buff, &msgPtr, sizeof(char*));

			//write message to hlp
			if(hlpID == 0 || hlpID == 1){
				//0 not valid, 1 is ethernet, not "above" anyone
			 fprintf(stderr, "Incorrect hlp: in Proto %d to %d:\n", protocolID, hlpID);
			 continue;
			}

			if(write(hlpPipe[hlpID-1], buff, sizeof(char*)) <0){
				fprintf(stderr, "in Proto %d to %d:\n", protocolID, hlpID );
				perror("write");
				exit(1);
			}
		}
	}

	void ppETH::sendUDP(int fd, char* message, int length){
	    // printf("Sending message: <<%s>>\n", message);
	    int totalBytesSent = 0;
	    while(totalBytesSent < length){
	        int bytesSent = write(fd, &message[totalBytesSent], length - totalBytesSent);
	        if(bytesSent == -1){
	            perror("ppETH: sendUDP write");
	            exit(1);
	        }
	        totalBytesSent += bytesSent;
	    }
	}
	template <class Header>
	ppAPP<Header>::ppAPP(int id, bool timer) : ppProtocol<Header>(id){
		// start threads in 'start funciton'
		//only recv pipe
		//still calling default.. awkward.
		if(pipe(this->recvPipe)<0){
			perror("pipe");
			exit(1);
		}
		//if true do signaling when app finished recv/send
		this->timing = timer;

	}
	template <class Header>
	void ppAPP<Header>::startApplication(){
		int rc;
		rc = pthread_create(&(this->sendThread),NULL, &handleSendHelper,this);
		if(rc){
		    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
			exit(1);	
		}
		rc = pthread_create(&(this->recvThread),NULL, &handleRecvHelper,this);
		if(rc){
		    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
			exit(1);	
		}
	}

	template <class Header>
	void ppAPP<Header>::startListen(){
		int rc;
		rc = pthread_create(&(this->recvThread),NULL, &handleRecvHelper,this);
		if(rc){
		    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
			exit(1);	
		}
	}

	template <class Header>
	void* ppAPP<Header>::handleSend(){
		// reads send pipe

		//generate 100 100 byte messages
		//send to llp
		//sleep

		int numMsgs = 100;
		int msgLen = 100;

		//sleep time in microseconds
		//5000 mircoseconds = 5 milliseconds
		useconds_t sleepTime = 5000;

		int packetSize = sizeof(int)+sizeof(char*);
		char buff[packetSize];
		int ptr_size = sizeof(char*);
		int int_size = sizeof(int);
		Message* msgPtr;
		Header* head;
		char* msgContent;
		for(int i = 1; i <= numMsgs; ++i){
			//create message 
			msgContent = new char[msgLen];
			memset(msgContent, 'A', msgLen);
			msgPtr = new Message(msgContent, msgLen);
			
			//create new header
			head = new Header;
			memset(head, 0, sizeof(*head));

			//end of the line, id apps by protoid of llp
			//(since llp won't be registered with self)
			head->hlp = this->protocolID;
			head->len = msgPtr->msgLen();

			//add header to message
			msgPtr->msgAddHdr((char*)head, sizeof(*head));

			//send message down pipe
			   //include this id (hlp)
			memcpy(&buff, &(this->protocolID), int_size);
			memcpy(&buff[int_size], &msgPtr, ptr_size);

			//write message to llp
			if(write(this->llpPipe, buff, packetSize)<0){
				perror("inapp: write");
				exit(1);
			}
			//sleep 
			usleep(sleepTime);
		}
		// printf("All messages sent\n");
		if(this->timing){
			pthread_mutex_lock(&count_mutex);
			count++;
			if(count >= numApps*2){
				pthread_cond_signal(&count_threshold_cv);
			}
			pthread_mutex_unlock(&count_mutex);
		}
		return NULL;
	}

	template <class Header>
	void* ppAPP<Header>::handleRecv(){
		//reads recv pipe, which receives from llp
		//count recvd messages

		int numMsgs = 100;
		int msgLen = 100;
		int nbytes;
		//expect msg ptr
		int packetSize = sizeof(char*);
		char buff[packetSize];
		int hlpID;
		Message* msgPtr;
		Header* head;
		char* msgContent;


		for(int i = 1; i <= numMsgs; i++){
			//block here until something is written
			//write is ATOMIC so we don't need mutices
			if((nbytes = read(this->recvPipe[0], buff, packetSize))<0){
				perror("read");
				exit(1);
			}
			else if(nbytes < packetSize){
				fprintf(stderr, "Bytes read less than expected, big trouble.\n");
			}
			//process received info
			memcpy(&msgPtr, &buff, packetSize);

			//remove own level protocol
			head = (Header*)msgPtr->msgStripHdr(sizeof(Header));
			hlpID = head->hlp;
			if(hlpID != this->protocolID){
				fprintf(stderr, "hlpID wrong in APP, ruh roh.\n");
			}

			msgContent = new char[msgLen];
			msgPtr->msgFlat(msgContent);
			msgContent[msgPtr->msgLen()] = '\0';

			// printf("Received msg #%d, %s\n", i,msgContent);

			//inadequate clean up
			delete head;
			delete[] msgContent;
			delete msgPtr;
		}
		
		// printf("All messages recvd\n");
		if(this->timing){
			pthread_mutex_lock(&count_mutex);
			count++;
			if(count >= numApps*2){
				pthread_cond_signal(&count_threshold_cv);
			}
			pthread_mutex_unlock(&count_mutex);
		}

		return NULL;
	}
	#endif