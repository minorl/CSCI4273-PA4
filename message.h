#ifndef __MESSAGE_H
#define __MESSAGE_H
#include <stdio.h>
#include <list>
#include <string.h>
#include <mutex>
using namespace std;

// extern std::atomic<int> count;
extern int count;
extern int numMsg;
extern pthread_mutex_t count_mutex;
extern pthread_cond_t count_threshold_cv;

class Message {
public:
   
    Message( );
    Message(char* msg, size_t len);
    ~Message( );
    void msgAddHdr(char *hdr, size_t length);
    char* msgStripHdr(int len);
    int msgSplit(Message& secondMsg, size_t len);
    void msgJoin(Message& secondMsg);
    size_t msgLen( );
    void msgFlat(char *buffer);
    char* originalBuffer;
    static int messageCount;
    int id;
    static int maxMessages;
    static mutex m;
    static bool done;
    static bool getDone();
    static void setDone(bool set);

private:
    size_t msglen;
    class field{
        public:
            field(char* data, size_t len){
                this->data  = data;
                this->len   = len;
            }
            char* data;
            size_t len;
    };
    list<field*> fieldList;
};
#endif
