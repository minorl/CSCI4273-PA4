#include "message.h"
#include "ppProtocol.h"

int main(int argc, char* argv[], char* envp[]){
	//silence warnings
	(void)argc;
	(void)argv;
	(void)envp;
	printf("setup\n");
	//	set up network
	ppETH eth(1, 6666, "5555", "127.0.0.1");
	ppIP ip(2);
	ppTCP tcp(3);
	ppUDP udp(4);
	ppFTP ftp(5);
	ppTEL tel(6);
	ppRDP rdp(7);
	ppDNS dns(8);

	ip.start();
	tcp.start();
	udp.start();
	ftp.start();
	tel.start();
	rdp.start();
	dns.start();

	ftpAPP ftpApplication(5);
	telAPP telApplication(6);
	rdpAPP rdpApplication(7);
	dnsAPP dnsApplication(8);

	eth.registerHLP(ip);
	ip.registerHLP(tcp);
	ip.registerHLP(udp);
	tcp.registerHLP(ftp);
	tcp.registerHLP(tel);	
	udp.registerHLP(rdp);
	udp.registerHLP(dns);

	dns.registerHLP(dnsApplication);
	ftp.registerHLP(ftpApplication);
	rdp.registerHLP(rdpApplication);
	tel.registerHLP(telApplication);
	
	ftp.registerLLP(tcp);
	tel.registerLLP(tcp);
	rdp.registerLLP(udp);
	dns.registerLLP(udp);
	tcp.registerLLP(ip);
	udp.registerLLP(ip);
	ip.registerLLP(eth);

	dnsApplication.registerLLP(dns);
	ftpApplication.registerLLP(ftp);
	rdpApplication.registerLLP(rdp);
	telApplication.registerLLP(tel);


	printf("setup done, starting apps in 5\n");
	sleep(5);
	dnsApplication.start();
	ftpApplication.start();
	rdpApplication.start();
	telApplication.start();
	// dnsApplication.start();
	// ftpApplication.start();
	// rdpApplication.start();
	// telApplication.start();
	sleep(600);
}