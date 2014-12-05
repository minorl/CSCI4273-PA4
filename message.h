#ifndef MESSAGE_H
#define MESSAGE_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>

struct messageNode
{
	char* msgptr;
	size_t len;
};

class Message
{
public:
   
    Message( );
    Message(char* msg, size_t len);
    ~Message( );
    void msgAddHdr(char *hdr, size_t length);
    char* msgStripHdr(size_t len);
    int msgSplit(Message& secondMsg, size_t len);
    void msgJoin(Message& secondMsg);
    size_t msgLen( );
    void msgFlat(char *buffer);

private:
	std::list<struct messageNode>::iterator findN(size_t &n);
    size_t msglen;
    std::list<struct messageNode> messageList;
};

#endif