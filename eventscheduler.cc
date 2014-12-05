#include "eventscheduler.h"

EventScheduler::EventScheduler(size_t maxEvents){
	die = false;
	pthread_mutex_init(&queueMutex, NULL);
	maxE = maxEvents;
	nextid = 1;

	signal(SIGUSR1, catch_alarm);

	sigset_t mask;  	
  	sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &oldset);
	pthread_create(&scheduler, NULL, &EventScheduler::work_helper, this);


}
EventScheduler::~EventScheduler(){
	die = true;
	pthread_join(scheduler, NULL);
	pthread_mutex_destroy(&queueMutex);

}
int EventScheduler::eventSchedule(void evFunction(void *), void* arg, int timeout){
	if(eventQueue.size() >= maxE){
		//no more things can be queued
		return -1;
	}
	bool recalc = false;
	struct timespec runtime;
	//get current time
	clock_gettime(CLOCK_MONOTONIC, &runtime);
	//add timeout (assume ms)
	addms2ts(&runtime, timeout);
	//create event
	struct Event newEvent;
	newEvent.id = nextid;
	newEvent.runtime = runtime;
	newEvent.func.dFunc = evFunction;
	newEvent.func.arg = arg;
	//add to queue
	pthread_mutex_lock(&queueMutex);
	if(!eventQueue.empty() && eventCompare(newEvent, eventQueue.top())){
		recalc = true;
	}
	eventQueue.push(newEvent);
	pthread_mutex_unlock(&queueMutex);

	if(recalc){
		//new event before first scheduled
		//need to recalculate 
		pthread_kill(scheduler, SIGUSR1);
	}
	//in the ghettoooo

	nextid++;
	return newEvent.id;
}
void EventScheduler::eventCancel(int eventId){
	pthread_mutex_lock(&queueMutex);
	// we don't care if top element is removed
	// thread checks top to make sure it's right time
	// recalc if not
	eventQueue.remove(eventId);

	pthread_mutex_unlock(&queueMutex);

}
void* EventScheduler::threadWork(){

	struct timespec tv;
	int retval;
	time_t defaultWait = 5;
	long defaultNano = 0;
	int waiting_on_id = 0;
	bool empty = false;

	//  //signal handling stuff
 //  	signal (SIGUSR1, catch_alarm);

 //  	//blcok signusr until in pselect
	// sigset_t mask, oldset;  	
 //  	sigemptyset(&mask);
 //    sigaddset(&mask, SIGUSR1);
 //    sigprocmask(SIG_BLOCK, &mask, &oldset);

	/* Wait up to five seconds. */
	tv.tv_sec = defaultWait;
	tv.tv_nsec = defaultNano;
  	
	while(!die || !empty){

		retval = 0;

		retval = pselect(0, NULL, NULL, NULL, &tv, &oldset);
		pthread_mutex_lock(&queueMutex);

		if(retval < 0){
          	//signal interrupt - recalc wait
          	// printf("SIGNALLED RECALCULATING\n");
		}
		else if(!eventQueue.empty() && eventQueue.top().id == waiting_on_id){

			tp.dispatch_thread(eventQueue.top().func.dFunc, eventQueue.top().func.arg);
	  		eventQueue.pop();

		}
		//recalc if wrong or successful
		// iterate through until we find something that needs to be run
		// and sets tv
		while(!eventQueue.empty() && (calcwait(tv, eventQueue.top().runtime)< 0)){
      		//If it's past time for this to run, run it nao.
      		tp.dispatch_thread(eventQueue.top().func.dFunc, eventQueue.top().func.arg);
      		eventQueue.pop();

         }

 		
		if(eventQueue.empty()){
			tv.tv_sec = defaultWait;
			tv.tv_nsec = defaultNano;
			waiting_on_id = 0;          		
    	}
        else{
        	//sscuessfully found next to be run
          	waiting_on_id = eventQueue.top().id;
         }
		empty = eventQueue.empty();        
		pthread_mutex_unlock(&queueMutex);
		// printf("q contains\n");
		// eventQueue.print();

	}
	// printf("thread exiting\n");
	return 0;
}
int EventScheduler::calcwait(struct timespec &wait, const struct timespec until){
	clock_gettime(CLOCK_MONOTONIC, &wait);
	// Okay, this is literally the worst.
	double waitnsec = wait.tv_sec * 1000000000 + wait.tv_nsec;
	double untilnsec = until.tv_sec * 1000000000 + until.tv_nsec;
	double diff = untilnsec - waitnsec;
	// printf("diff is %f\n", diff);
	if(diff > 0){
		wait.tv_sec = diff / 1000000000;
		wait.tv_nsec = ((long)diff) % 1000000000;
	}
	else{
		//should run asap.
		return -1;
	}
	return 1;
}
bool EventScheduler::eventCompare(const struct Event& lhs, const struct Event&rhs){
    // if (lhs.runtime.tv_sec < rhs.runtime.tv_sec)
    //     return true ;				/* Less than. */
    // else if (lhs.runtime.tv_sec > rhs.runtime.tv_sec)
    //     return false ;				 /*Greater than.*/
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

void EventScheduler::addms2ts(struct timespec *ts, int ms)
{
    ts->tv_sec += ms / 1000;
    ts->tv_nsec += (ms % 1000) * 1000000;
}

void EventScheduler::catch_alarm (int sig){
  	signal(sig, catch_alarm);
}
