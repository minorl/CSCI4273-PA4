ppProtocol::ppProtocol(){
	// FD_ZERO(&sendFDset);
	// FD_ZERO(&recvFDset);


}

ppProtocol::~ppProtocol(){

}

ppProtocol::registerSendProtocol(int protocolID, int pipeFD, pthread_mutex_t pipeMutex){
	sendPipe[protocolID-1] = pipeFD;
	sendMutex[protocolID-1] = pipeMutex;
	FD_SET(pipeFD, &sendFDset);
}

ppProtocol::registerRecvProtocol(int protocolID, int pipeFD, pthread_mutex_t pipeMutex){
	recvPipe[protocolID-1] = pipeFD;
	recvMutex[protocolID-1] = pipeMutex;
	FD_SET(pipeFD, &recvFDset);
}
void ppProtocol::registerHLP(ppProtocol hlp){
	//this level of abstraction is unncessary
	// but it's so confusing
	//registers a higher level protocol
	// hlp.getRecv(hlpPipe, hlpMutex);
	hlp.getRecv(hlpPipe);
}
void ppProtocol::registerLLP(ppProtocol llp){
	//registers a lower level protocol
	llp.getSend(llpPipe);
}
void ppProtocol::getRecv(int pipe[]){
	//passes own information to a llp trying to register
	// so send recv values
	pipe[protocolID-1] = recvPipe;
	// mutex[protocolID-1] = &recvMutex;
}
void ppProtocol::getSend(int *pipe){
	//passes own information to a hlp trying to register
	// so send values
	*pipe = sendPipe;
}
ppProtocol::handleSend(){
	// reads send pipe
	// which receives from hlp
	// handles outbound messages
	// send ot llp
	int nbytes;
	//expect int + msg ptr
	int packetSize = sizeof(int)+sizeof(char*);
	char buff[packetSize];

	int hlpID;
	Message* msgPtr;
	Header* head;
	while(true){
		// if(select(FD_SETSIZE, &sendFDset, NULL, NULL, NULL) < 0){
		// 	perror("select");
		// 	exit(1);
		// }
		//All registered fd's best be in our array
		// for(i = 0; i < NUM_PROTOCOLS; ++i){
		// 	if(FD_ISSET(sendPipe[i], &sendFDset)){
		// 		pthread_mutex_lock(&sendMutex[i]);
		// 			//read protocol id and msg ptr
		// 		pthread_mutex_unlock(&sendMutex[i]);
		// 	}
		// }
		if((nbytes = read(sendPipe, buff, receiveSize))<0){
			perror("read");
			exit(1)
		}
		else if(nbytes < packetSize){
			fprintf(stderr, "Bytes read less than expected, big trouble.\n");
		}
		//process received info
		hlpID = *((int*)buff);
		msgPtr = (Message*)(buff+sizeof(int));
		//create new header
		head = new Header;
		head->hlp = hlpID;
		head->len= msgPtr->msgLen();
		//add header to message
		msgPtr->msgAddHeader((char*)head, sizeof(*head));
		//send message down pipe
		*((int*)buff) = protocolID;
		//it should already be this
		(Message*)(buff+sizeof(int)) = msgPtr;

		write(llp, buff. packetSize);
	}

		
}
