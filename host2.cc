#include "message.h"
#include "ppProtocol.h"

int main(int argc, char* argv[], char* envp[]){
	//silence warnings
	(void)argc;
	(void)argv;
	(void)envp;
	printf("setup\n");
	//	set up network
	ppETH eth(6666, "5555", "127.0.0.1");
	ppIP ip;
	ppTCP tcp;
	ppUDP udp;
	ppFTP ftp;
	ppTEL tel;
	ppRDP rdp;
	ppDNS dns;

	eth.setID(1);
	ip.setID(2);
	tcp.setID(3);
	udp.setID(4);
	ftp.setID(5);
	tel.setID(6);
	rdp.setID(7);
	dns.setID(8);

	eth.registerHLP(ip);
	ip.registerHLP(tcp);
	ip.registerHLP(udp);
	tcp.registerHLP(ftp);
	tcp.registerHLP(tel);	
	udp.registerHLP(rdp);
	udp.registerHLP(dns);
	
	ftp.registerLLP(tcp);
	tel.registerLLP(tcp);
	rdp.registerLLP(udp);
	dns.registerLLP(udp);
	tcp.registerLLP(ip);
	udp.registerLLP(ip);
	ip.registerLLP(eth);

	printf("setup done\n");
	// simulate app
	int appfd[2];
	// int socketfd[2];

	if(pipe(appfd)<0){
		perror("pipe");
	}
	// if(pipe(socketfd)<0){
	// 	perror("pipe");
	// }

	//ftp will write up to app
	ftp.hlpPipe[1] = appfd[1];
	// app will write to ftp sendpipe

	// //make eth write down to socket, we'll read parse and write back
	// eth.llpPipe = socketfd[1];


	printf("making message\n");
	int x = 2;
	char* thing = "hello world";
	Message *m = new Message(thing, 12);
	char buff[sizeof(char*)+sizeof(x)];
	char buff2[sizeof(char*)+sizeof(x)];
	memcpy(&buff, &x, sizeof(x));
	memcpy(&buff[sizeof(x)], &m, sizeof(char*));

	int nbytes;
	printf("write\n");
	sleep(2);
	nbytes = write(ftp.sendPipe[1], buff, sizeof(char*)+sizeof(x));
	// printf("readat bottom\n");
	// nbytes = read(socketfd[0], buff,sizeof(char*)+sizeof(x));
	// printf("write back bottom\n");
	// nbytes = write(eth.recvPipe[1], &buff[sizeof(x)], sizeof(char*));
	printf("read at top\n");
	nbytes = read(appfd[0], buff2, sizeof(char*));
	Message *n;
	memcpy(&n, buff2, sizeof(char*));
	char thing2[12];
	n->msgFlat(thing2);
	thing2[n->msgLen()] ='\0';
	printf("%s\n", thing2);
	sleep(10);

}