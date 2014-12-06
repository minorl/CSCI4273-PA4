#include "ppProtocol.h"

ppProtocol::ppProtocol(){
	int rc;
	rc = pthread_create(&sendThread,NULL, &ppProtocol::handleSendHelper,this);
	if(rc){
	    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
		exit(1);	
	}
	rc = pthread_create(&recvThread,NULL, &ppProtocol::handleRecvHelper,this);
	if(rc){
	    fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
		exit(1);	
	}
	protocolID=1;
	if(pipe(sendPipe)<0){
		perror("pipe");
		exit(1);
	}
	if(pipe(recvPipe)<0){
		perror("pipe");
		exit(1);
	}
	printf("sendpipe 0,1: %d %d\n", sendPipe[0],sendPipe[1]);

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
	pipe[protocolID-1] = recvPipe[1];
	// mutex[protocolID-1] = &recvMutex;
}
void ppProtocol::getSend(int *pipe){
	//passes own information to a hlp trying to register
	// so send values
	//give WRITE end of send pipe
	*pipe = sendPipe[1];
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
	Header tester;
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
		if(write(hlpPipe[hlpID-1], buff, packetSize) <0){
			perror("write");
			exit(1);
		}
	}
}