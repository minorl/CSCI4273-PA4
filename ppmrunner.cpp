#include "ppm.h"
#include <iostream>
#include <ctime>
#include <time.h>
#include <sys/time.h>

//timing stuff
int numMsg = 400;
int count = 0;
// std::atomic<int> count;
pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;

void breakTime(int seconds);
double diffms(timeval t1, timeval t2);

int main(int argc, char** argv){
    if(argc < 4){
        printf("Not enough args: %s <local port> <remote host> <remote port>\n", argv[0]);
        exit(1);
    }
    char* port = argv[1];
    char* remoteHost = argv[2];
    char* remotePort = argv[3];
    ProcessPerMessage * ppm = new ProcessPerMessage(port, remoteHost, remotePort);
   
    //timing set up
    pthread_mutex_init(&count_mutex, NULL);
    pthread_cond_init (&count_threshold_cv, NULL);

    struct timeval t1;
    struct timeval t2;


    sleep(7);
    gettimeofday(&t1, NULL);
    // printf("Going.\n");
    for(int i = 0;i < 100;i++){
        // printf("Sending message block #%d\n", i);
        char* message = new char[9];
        strcpy(message,"HelloDNS");
        ppm->sendTopLevel(message, 9, 8);
        // breakTime(5);
        message = new char[9];
        strcpy(message,"HelloRDP");
        ppm->sendTopLevel(message, 9, 7);
        // breakTime(5);
        message = new char[9];
        strcpy(message,"HelloTel");
        ppm->sendTopLevel(message, 9, 6);
        // breakTime(5);
        message = new char[9];
        strcpy(message,"HelloFTP");
        ppm->sendTopLevel(message, 9, 5);
        breakTime(5);
    }

    // printf("Done sending\n");
    //wait for apps to finish and then stop timing or whatever
    pthread_mutex_lock(&count_mutex);
    while (count<numMsg) {
        pthread_cond_wait(&count_threshold_cv, &count_mutex);
    }
    pthread_mutex_unlock(&count_mutex);
    gettimeofday(&t2, NULL);
    printf("Listening on %s took %f ms\n", port, diffms(t2,t1));

    // while(!Message::getDone()){}
}

void breakTime(int milliseconds){
        clock_t temp;
            temp = clock () + milliseconds * CLOCKS_PER_SEC/1000 ;
                while (clock() < temp) {}
}
double diffms(timeval t1, timeval t2)
{
    double diffsec = (t1.tv_sec - t2.tv_sec);
    double diffusec = (t1.tv_usec - t2.tv_usec);
    return diffsec * 1000.0 +  diffusec* .001;
}