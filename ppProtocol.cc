#include "ppProtocol.h"

ppProtocol::ppProtocol(){
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
	if(pipe(sendPipe)<0){
		perror("pipe");
		exit(1);
	}
	if(pipe(recvPipe)<0){
		perror("pipe");
		exit(1);
	}
}

ppProtocol::~ppProtocol(){
	//leak baby leak
}

void ppProtocol::registerHLP(ppProtocol hlp){
	//this level of abstraction is unncessary
	// but it's so confusing the naming conventions
	//registers a higher level protocol
	// hlp.getRecv(hlpPipe, hlpMutex);
	hlp.getRecv(hlpPipe);
}
void ppProtocol::registerLLP(ppProtocol llp){
	//registers a lower level protocol
	llp.getSend(&llpPipe);
}
void ppProtocol::getRecv(int pipe[]){
	//passes own information to a llp trying to register
	// so send recv values
	//give WRITE end of recv pipe
	printf("setting proto id: %d to fd %d\n", protocolID-1, recvPipe[1] );
	pipe[protocolID-1] = recvPipe[1];
	// mutex[protocolID-1] = &recvMutex;
}
void ppProtocol::getSend(int *pipe){
	//passes own information to a hlp trying to register
	// so send values
	//give WRITE end of send pipe
	*pipe = sendPipe[1];
}
void ppProtocol::setID(int id){
	protocolID = id;
}
void* ppProtocol::handleSend(){
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
		}
		printf("PROTOID SENDING DOWN %d\n", protocolID);
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
		printf("WRITING MSG LEN %d\n", msgPtr->msgLen());
		//send message down pipe
		   //include this id (hlp)
		memcpy(&buff, &protocolID, int_size);
		// *((int*)buff) = protocolID;
		//it should already be this
		// *(Message*)(buff+sizeof(int)) = msgPtr;

		//write message to llp
		if(write(llpPipe, buff, packetSize)<0){
			perror("write");
			exit(1);
		}
	}
}

void* ppProtocol::handleRecv(){
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
		printf("blocked to read\n");
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
		//remove own level protocol
		head = (Header*)msgPtr->msgStripHdr(sizeof(Header));
		hlpID = head->hlp;

		//send message up pipe
		   //to appropriate hlp

		//this should already be set
		// (Message*)(buff) = msgPtr;

		//write message to hlp
		printf("protoid %d sending msg up to %d\n", protocolID, hlpID);

		if(write(hlpPipe[hlpID-1], buff, packetSize) <0){
			perror("write");
			exit(1);
		}
	}
}

ppETH::ppETH(int listenPort, const char* sendPort, const char* sendHost):ppProtocol(){
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
	if(pipe(sendPipe)<0){
		perror("pipe");
		exit(1);
	}
	//recv is from socket
	recvReadSocket = bindUDPListen(listenPort);
	recvSendSocket = getUDPSend(sendHost, sendPort);
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
	Header* head;
	char sendbuff[1024];
	int msgLen;
	while(true){
		//block here until something is written
		//write is ATOMIC so we don't need mutices
		nbytes = read(sendPipe[0], buff, packetSize);
		printf("ETH READ FROM SENDPIPE\n");
		if(nbytes<0){
			perror("read");
			exit(1);
		}
		else if(nbytes < packetSize){
			fprintf(stderr, "Bytes read less than expected, big trouble.\n");
		}
		printf("ETHERNET SENDING MSG\n");

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
	Header* head;
	char* charptr;



	while(true){
		//block here until something is written
		//write is ATOMIC so we don't need mutices
		printf("blocked to read\n");
		if((nbytes = read(recvReadSocket, buff, packetSize))<0){
			perror("read");
			exit(1);
		}
		else if(nbytes < packetSize){
			fprintf(stderr, "Bytes read less than expected, big trouble.\n");
		}
		//process received info
		memcpy(&msgLen, &buff, sizeof(int));
		msgLen = ntohl(msgLen);
		printf("msgLen is %d\n", msgLen);
		
		charptr = new char[msgLen];
		memset(charptr, 0, msgLen);
		memcpy(charptr, &buff[sizeof(int)], msgLen);
		printf("msg is %s\n", charptr);

		msgPtr = new Message(charptr, msgLen);
		//remove own level protocol
		head = (Header*)msgPtr->msgStripHdr(sizeof(Header));
		hlpID = head->hlp;
		
		memset(buff,0,packetSize);		
		memcpy(&buff, &msgPtr, sizeof(char*));

		//write message to hlp
		if(write(hlpPipe[hlpID-1], buff, sizeof(char*)) <0){
			perror("write");
			exit(1);
		}
	}
}

void ppETH::sendUDP(int fd, char* message, int length){
    printf("Sending message: <<%s>>\n", message);
    int totalBytesSent = 0;
    while(totalBytesSent < length){
        int bytesSent = write(fd, &message[totalBytesSent], length - totalBytesSent);
        if(bytesSent == -1){
            perror("sendUDP write");
            exit(1);
        }
        totalBytesSent += bytesSent;
    }
}

void* ppAPP::handleSend(){
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
	for(int i = 1; i <= msgLen; ++i){
		printf("APP SENDING DOWN %d\n", protocolID);
		//create message 
		msgContent = new char[msgLen];
		memset(msgContent, 'A', msgLen);
		msgPtr = new Message(msgContent);
		
		//create new header
		head = new Header;
		memset(head, 0, sizeof(*head));

		//end of the line
		head->hlp = 0;
		head->len = msgPtr->msgLen();

		//add header to message
		msgPtr->msgAddHdr((char*)head, sizeof(*head));
		printf("WRITING MSG LEN %d\n", msgPtr->msgLen());

		//send message down pipe
		   //include this id (hlp)
		memcpy(&buff, &protocolID, int_size);
		memcpy(&buff[int_size], &msgPtr, ptr_size);

		//write message to llp
		if(write(llpPipe, buff, packetSize)<0){
			perror("write");
			exit(1);
		}
		//sleep 
		usleep(sleepTime);
	}
}

void* ppProtocol::handleRecv(){
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

	for(int i = 1; i <= 100; i++){
		//block here until something is written
		//write is ATOMIC so we don't need mutices
		printf("blocked to read\n");
		if((nbytes = read(recvPipe[0], buff, packetSize))<0){
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

		msgContent = new char[100];
		msgPtr->msgFlat(msgContent);
		msgContent[msgPtr->msgLen()] = '\0';
		printf("App recv %s\n", msgContent);

		//inadequate clean up
		delete head;
		delete[] msgContent;
		delete msgPtr;
		if (i % 10 == 0)
		{
			printf("%d messages recvd\n", i);
		}
	}
}