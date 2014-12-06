#include "message.h"
#include "ppProtocol.h"

int main(){
	ppTCP a;
	char* testptr;
	testptr = new char[24];
	memset(testptr, 0, 24);

	int hlp[2];
	int llp[2];
	int nbytes;
	int downPacket = sizeof(int)+sizeof(char*);
	int upPacket = sizeof(char*);
	char *b1 = new char[5];
    for (int i = 0; i < 5; i++) b1[i] = 'h';
    Message *m = new Message(b1, 5);
	printf("Message pointer is %p\n", m);
	Message *n;
	char buff[downPacket];
	char buff2[downPacket];
	char buff3[upPacket];

	//hate errors
	int ptr_size, int_size;
	ptr_size = sizeof(char*);
	int_size = sizeof(int);

	//protocol id
	int x = 2;
	memcpy(&buff, &x, int_size);
	memcpy(&buff[int_size], &m, ptr_size);


	if(pipe(hlp)<0){
		perror("pipe");
	}
	if(pipe(llp)<0){
		perror("pipe");
	}
	a.llpPipe = llp[1];
	a.hlpPipe[1] = hlp[1];

	nbytes = write(a.sendPipe[1], buff, sizeof(int)+sizeof(char*));
	if(nbytes < 0){
		perror("write");
		exit(1);
	}
	nbytes = read(llp[0], buff2, downPacket);
	if(nbytes < 0){
		perror("read");
		exit(1);
	}
	nbytes = write(a.recvPipe[1], &buff[4], upPacket);
	if(nbytes < 0){
		perror("write");
		exit(1);
	}
	nbytes = read(hlp[0], buff3, upPacket);
	printf("%d read\n", nbytes);
	if(nbytes < 0){
		perror("read");
		exit(1);
	}

	memcpy(&n, &buff3, sizeof(char*)); 
	printf("m: %p, n: %p \n", m, n);


	// a.registerHLP(b);
	// a.registerLLP(b);
	// b.registerHLP(a);
	// b.registerLLP(b);
}