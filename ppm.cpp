    #include "ppm.h"

    ProcessPerMessage::ProcessPerMessage(char* port, char* remoteHost, char* remotePort){
        tp = new ThreadPool(25);
        receiverArgs * rArgs = new receiverArgs;
        rArgs->tp = tp;
        rArgs->port = port;
        rArgs->remoteHost = remoteHost;
        rArgs->remotePort = remotePort;
        outSockFD = getUDPSend(remoteHost, remotePort);
        pthread_create(&receiverThread, NULL, &receiver, rArgs);
    }

    ProcessPerMessage::~ProcessPerMessage(){
        delete tp;
    }

    void* ProcessPerMessage::receiver(void* args){
        receiverArgs * rArgs = (receiverArgs*)args;
        int sockFD = bindUDPListen(atoi(rArgs->port));
        ThreadPool * tp = rArgs->tp;
        while(1){
            char readBuf[1024];
            //printf("Waiting...\n");
            recv(sockFD, readBuf, 1024, 0);
            int msgLen = ntohl(*((int*) readBuf));
            Message * inboundMsg = new Message(readBuf + sizeof(int), msgLen);
            tp->dispatch_thread(deliverEthernet, (void*)inboundMsg);
        }
        //If I add a termination condition, delete rArgs here
        return NULL;
    }

    void ProcessPerMessage::deliverEthernet(void* m){
        // printf("Delivering ethernet.\n");
        Message * inboundMsg = (Message*) m;
        char* header = inboundMsg->msgStripHdr(sizeof(int) + 8 + sizeof(size_t));
        int hlpId = *((int*)header);
        if(hlpId == 2){
            deliverIP(inboundMsg);
        }else{
            printf("Ethernet received wrong hlp.");
        }
    }

    void ProcessPerMessage::deliverIP(Message* inboundMsg){
        // printf("Delivering ip.\n");
        //printf("Stripping header\n");
        char* header = inboundMsg->msgStripHdr(sizeof(int) + 12 + sizeof(size_t));
        //printf("Stripped header\n");
        fflush(stdout);
        int hlpId = *((int*)header);
        if((hlpId == 3) || (hlpId == 4)){
            deliverTransportLayer(inboundMsg);
        }else{
            printf("IP received wrong hlp.");
        }
    }

    void ProcessPerMessage::deliverTransportLayer(Message* inboundMsg){
        // printf("Delvering transport.\n");
        char* header = inboundMsg->msgStripHdr(sizeof(int) + 4 + sizeof(size_t));
        int hlpId = *((int*)header);
        switch(hlpId){
        case 5:
            deliverTopLevel(inboundMsg, 8);
            break;
        case 6:
            deliverTopLevel(inboundMsg, 8);
            break;
        case 7:
            deliverTopLevel(inboundMsg, 12);
            break;
        case 8:
            deliverTopLevel(inboundMsg, 8);
            break;
        default:
            printf("Incorrect hlp in TCP/UDP: got %d.\n", hlpId);
        }

        }

    void ProcessPerMessage::deliverTopLevel(Message* inboundMsg, int strip){
        // printf("Delvering top level.\n");
        char* header = inboundMsg->msgStripHdr(sizeof(int) + strip + sizeof(size_t));
        int hlpId = *((int*)header);
        switch(hlpId){
        case 5:
            deliverToApp(inboundMsg, 8, "FTP");
            break;
        case 6:
            deliverToApp(inboundMsg, 8, "Telnet");
            break;
        case 7:
            deliverToApp(inboundMsg, 12, "RDP");
            break;
        case 8:
            deliverToApp(inboundMsg, 8, "DNS");
            break;
        default:
            printf("Incorrect hlp in top level: got %d.\n", hlpId);
        }
    }

    

    void ProcessPerMessage::deliverToApp(Message* inboundMsg, int strip, const char* name){
        // printf("Delivering to app.\n");
        inboundMsg->msgStripHdr(sizeof(int) + strip + sizeof(size_t));
        size_t length = inboundMsg->msgLen();
        char * remainder = new char[length + 1];
        inboundMsg->msgFlat(remainder);
        remainder[length] = '\0';
        // printf("%s got message #%d: %s\n", name, inboundMsg->id, remainder);
        fflush(stdout);
        (void)name;

        // this will probably slow it down a lot...
        // it does. 
        pthread_mutex_lock(&count_mutex);
        ++count;
        if(count >= 400){
            pthread_cond_signal(&count_threshold_cv);
        }
        pthread_mutex_unlock(&count_mutex);

    }

    void ProcessPerMessage::sendTopLevel(char* message, size_t length, int protoId){
        //printf("Sending top level. ID: %d\n",protoId);
        Message * outboundMessage = new Message(message, length);
        //RDP has 12 byte infolength
        int infoLength;
        if(protoId == 7){
            infoLength = 12;
        }else{
            infoLength = 8;
        }

        int transportProtoId;
        if((protoId == 5) || (protoId == 6)){
            transportProtoId = 3;
        }else{
            transportProtoId = 4;
        }

        size_t headerLength = sizeof(int) + infoLength + sizeof(size_t);
        // printf("Protoid %d header length: %lu\n",protoId, headerLength);
        char* header = new char[headerLength];
        *((int*)header) = protoId;
        memset(header + sizeof(int), 0, infoLength);
        *((size_t*)(header + sizeof(int) + infoLength)) = outboundMessage->msgLen();

        //double header to match PPP code
        outboundMessage->msgAddHdr(header, headerLength);
        char* header2 = new char[headerLength];
        memcpy(header2, header, headerLength);
        *((size_t*)(header + sizeof(int) + infoLength)) = outboundMessage->msgLen();
        outboundMessage->msgAddHdr(header2, headerLength);

        sendTransportLayer(outboundMessage, transportProtoId, protoId);
    }

    void ProcessPerMessage::sendTransportLayer(Message * outboundMessage,int protoId, int hlp){
        int infoLength = 4;
        size_t headerLength = sizeof(int) + infoLength + sizeof(size_t);
        // printf("UDP/TCP header length: %lu\n", headerLength);
        char* header = new char[headerLength];
        *((int*)header) = hlp;
        memset(header + sizeof(int), 0, infoLength);
        *((size_t*)(header + sizeof(int) + infoLength)) = outboundMessage->msgLen();
        outboundMessage->msgAddHdr(header, headerLength);
        sendIP(outboundMessage, protoId);
    }

    void ProcessPerMessage::sendIP(Message * outboundMessage, int hlp){
        int infoLength = 12;
        size_t headerLength = sizeof(int) + infoLength + sizeof(size_t);
        // printf("IP header length: %lu\n", headerLength);
        char* header = new char[headerLength];
        *((int*)header) = hlp;
        memset(header + sizeof(int), 0, infoLength);
        *((size_t*)(header + sizeof(int) + infoLength)) = outboundMessage->msgLen();
        outboundMessage->msgAddHdr(header, headerLength);
        sendEthernet(outboundMessage);
    }

    void ProcessPerMessage::sendEthernet(Message * outboundMessage){
        int infoLength = 8;
        size_t headerLength = sizeof(int) + infoLength + sizeof(size_t);
        char* header = new char[headerLength];
        *((int*)header) = 2; //IP is the hlp
        memset(header + sizeof(int), 0, infoLength);
        *((size_t*)(header + sizeof(int) + infoLength)) = outboundMessage->msgLen();
        // printf("Eth header len %lu\n", headerLength);
        outboundMessage->msgAddHdr(header, headerLength);
        char * data = new char[1024];
        memset(data,0,1024);
        outboundMessage->msgFlat(data + sizeof(int));
        *((int*)data) = htonl(outboundMessage->msgLen());



        sendUDP(outSockFD, data, 1024);
    }
