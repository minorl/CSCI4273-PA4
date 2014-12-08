#include "message.h"
#include "ppp.h"
#include <time.h>
#include <sys/time.h>

int numApps = 4;
int count = 0;
pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;

double diffms(timeval t1, timeval t2)
{
	double diffsec = (t1.tv_sec - t2.tv_sec);
	double diffusec = (t1.tv_usec - t2.tv_usec);
    return diffsec * 1000.0 +  diffusec* .001;
}
int main(int argc, char* argv[], char* envp[]){
	//silence warnings
	(void)envp;
    if(argc != 4){
        printf("Wrong number of arguments. Usage %s <local port> <remote host> <remote port> \n", argv[0]);
        return 1;
    }

	int listenPort = atoi(argv[1]);

	//timing synchronization
	//apps will signal when they're all done

	pthread_mutex_init(&count_mutex, NULL);
  	pthread_cond_init (&count_threshold_cv, NULL);

  	struct timeval t1;
  	struct timeval t2;

	//	set up network
	ppETH eth(1, listenPort, argv[3], argv[2]);
	ppIP ip(2);
	ppTCP tcp(3);
	ppUDP udp(4);
	ppFTP ftp(5);
	ppTEL tel(6);
	ppRDP rdp(7);
	ppDNS dns(8);

	ftpAPP ftpApplication(5, true);
	telAPP telApplication(6, true);
	rdpAPP rdpApplication(7, true);
	dnsAPP dnsApplication(8, true);

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

	ip.start();
	tcp.start();
	udp.start();
	ftp.start();
	tel.start();
	rdp.start();
	dns.start();


	//make sure everything is set before we start timing
	sleep(5);
	gettimeofday(&t1, NULL);
	// dnsApplication.startListen();
	// ftpApplication.startListen();
	// rdpApplication.startListen();
	// telApplication.startListen();
	dnsApplication.startApplication();
	ftpApplication.startApplication();
	rdpApplication.startApplication();
	telApplication.startApplication();

	//wait for apps to finish and then stop timing or whatever
	pthread_mutex_lock(&count_mutex);
	// while (count<numApps*2) {
	while (count<numApps*2) {
		pthread_cond_wait(&count_threshold_cv, &count_mutex);
	}
	pthread_mutex_unlock(&count_mutex);
	gettimeofday(&t2, NULL);
	printf("Listening on %d took %f ms\n", listenPort, diffms(t2,t1));



}