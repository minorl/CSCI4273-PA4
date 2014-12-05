#ifndef EVENTSCHEDULER_H
#define EVENTSCHEDULER_H

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <queue>
#include <unistd.h>
#include <time.h>
#include "threadpool.h"



// struct FunctionInfo
// {
// 	void (*dFunc)(void*);
// 	void* arg;
// };
struct Event{
	int id;
	struct timespec runtime;
	struct FunctionInfo func;
};

template <class T, class C,class S > 
  class removable_priority_queue: public std::priority_queue<T,C,S>
  {
  public:
    void remove(int target){
	    for (unsigned i=0; i<this->c.size(); ++i){
	    	if(this->c[i].id == target){
	    		this->c.erase(this->c.begin() + i);
	    		break;
	    	}
	    }
    }
    void print(){
    	for (unsigned i=0; i<this->c.size(); ++i){
	    	printf("id: %d tsec: %zu\n", this->c[i].id, this->c[i].runtime.tv_sec);
	    }
    } 
}; 
class CompareEvent {
    public:
    bool operator()(const struct Event& lhs, const struct Event&rhs){
    // if (lhs.runtime.tv_sec < rhs.runtime.tv_sec)
    //     return true ;				/* Less than. */
    // else if (lhs.runtime.tv_sec > rhs.runtime.tv_sec)
    //     return false ;				/* Greater than. */
    // else if (lhs.runtime.tv_nsec < rhs.runtime.tv_nsec)
    //     return true ;				/* Less than. */
    // else if (lhs.runtime.tv_nsec > rhs.runtime.tv_nsec)
    //     return false;				/* Greater than. */
    // else
    //     return false;				/* Equal. */
    double lhsnsec = lhs.runtime.tv_sec * 1000000000 + lhs.runtime.tv_nsec;
    double rhsnsec = rhs.runtime.tv_sec * 1000000000 + rhs.runtime.tv_nsec;
    return lhsnsec > rhsnsec;
	}
};

class EventScheduler{
  public:
	EventScheduler(size_t maxEvents = 10);
	~EventScheduler();
	int eventSchedule(void evFunction(void *), void* arg, int timeout);
	void eventCancel(int eventId);

  private:
  	pthread_t scheduler;
  	sigset_t oldset;
	pthread_mutex_t queueMutex;
	removable_priority_queue<struct Event, std::vector<struct Event>, CompareEvent> eventQueue;
	bool die;
	size_t maxE;
	int nextid;
	void* threadWork(void);
	ThreadPool tp;
	static void *work_helper(void *instance) {
		return ((EventScheduler*)instance)->threadWork();
	}
	bool eventCompare(const struct Event& lhs, const struct Event&rhs);
	void addms2ts(struct timespec *ts, int ms);
	int calcwait(struct timespec &wait, const struct timespec until);
	static void catch_alarm (int sig);
};


#endif