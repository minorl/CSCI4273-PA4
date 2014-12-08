#ifndef __PROCESSSPERMESSAGE_H
#define __PROCESSSPERMESSAGE_H
#include <iostream>
#include <fstream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include "threadpool.h"
#include "message.h"
#include <pthread.h>
#include <mutex>
#include <atomic>


//timing stuff
// extern std::atomic<int> count;
extern int count;
extern int numMsg;
extern pthread_mutex_t count_mutex;
extern pthread_cond_t count_threshold_cv;

class ProcessPerMessage{
    public:
        ProcessPerMessage(char* port, char* remoteHost, char* remotePort);
        ~ProcessPerMessage();

        void sendTopLevel(char* message, size_t length, int protoId);
        int getNumMessages();
    private:
        void sendTransportLayer(Message * outboundMessage, int protoId, int hlp);
        void sendIP(Message * outboundMessage, int hlp);
        void sendEthernet(Message * outboundMessage);

        static void* receiver(void* args);
        static void deliverEthernet(void * m);
        static void deliverIP(Message* m);
        static void deliverTransportLayer(Message* m);
        static void deliverTopLevel(Message* m, int strip);
        static void deliverToApp(Message* m, int strip, const char* name);

        int outSockFD;

        typedef struct {
            ThreadPool * tp;
            char* port;
            char* remoteHost;
            char* remotePort;
        }receiverArgs;

        ThreadPool * tp;
        pthread_t receiverThread;
        static int getUDPSend(const char *host, const char *portnum){
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

        static int bindUDPListen(int port){
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

        void sendUDP(int fd, char* message, int length){
            // printf("Sending message: <<%s>>, length %d\n", message, length);
            int totalBytesSent = 0;
            while(totalBytesSent < length){
                int bytesSent = write(fd, &message[totalBytesSent], length - totalBytesSent);
                if(bytesSent == -1){
                    perror("ppm::sendUDP write");
                    exit(1);
                }
                totalBytesSent += bytesSent;
            }
        }

};
#endif
