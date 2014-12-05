#include "threadpool.h"

ThreadPool::ThreadPool(const size_t threadCount){
	/* Initilization */
	int rc;
	numThreads = threadCount;
	availableThreads = numThreads;
	die = false;
	threads = new pthread_t[numThreads];
	if(pthread_mutex_init(&queueMutex, NULL) ||
		pthread_mutex_init(&countMutex, NULL)){
		fprintf(stderr, "mutex init failed");
		exit(EXIT_FAILURE);	
	}
	//want everything blocking initially
	sem_init(&workSem, 0, 0);
	for(size_t i=0; i< numThreads; i++){
		//I don't know if this will create problems, from moodle, seems uberhacky
		rc = pthread_create(&(threads[i]), NULL, &ThreadPool::work_helper, this);
		if (rc){
			fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
			exit(EXIT_FAILURE);	
		}
	}
}
ThreadPool::~ThreadPool(){
	//wait for jobs to finish
	// if they don't finish, we're in trouble
	while(!workQueue.empty()){
		//this is not the best idea
		sleep(.5);
	}
	die = true;
	//Make sure no threads blocked still
	for(size_t i=0; i< numThreads; i++){
		sem_post(&workSem);
	}	
	for(size_t i=0; i< numThreads; i++){
		pthread_join(threads[i], NULL);
	}
	if(pthread_mutex_destroy(&queueMutex)
		||pthread_mutex_destroy(&countMutex))
		fprintf(stderr, "Mutex destroy fail\n");
	if(sem_destroy(&workSem))
		fprintf(stderr, "Semaphore destroy fail\n");
	delete[] threads;

}
int ThreadPool::dispatch_thread(void dispatch_function (void*),void *arg){
	struct FunctionInfo newJob;
	newJob.dFunc = dispatch_function;
	newJob.arg = arg;
	pthread_mutex_lock(&queueMutex);
	workQueue.push(newJob);
	pthread_mutex_unlock(&queueMutex);
	//Signal new work to be done
	//sem_post non zero on error, 
	return sem_post(&workSem);
}
bool ThreadPool::thread_avail(){
	int avail;
	// this is a stupid requirement.
	// really i'm stupid for putting in a queue
    pthread_mutex_lock(&countMutex);
    avail = availableThreads;
    pthread_mutex_unlock(&countMutex);
    return (bool)avail;
}
void* ThreadPool::threadWork(){
	struct FunctionInfo currentJob;
	while(true){
		sem_wait(&workSem);
		if(die){
			break;
		}
		pthread_mutex_lock(&countMutex);
    		availableThreads--;
    	pthread_mutex_unlock(&countMutex);
		pthread_mutex_lock(&queueMutex);
		if(workQueue.empty()){
			fprintf(stderr, "workQueue is empty\n");
			exit(EXIT_FAILURE);
		}
		currentJob = workQueue.front();
		workQueue.pop();
		pthread_mutex_unlock(&queueMutex);
		// foo = (void*)(currentJob.dFunc)(void*);
		currentJob.dFunc(currentJob.arg);
		pthread_mutex_lock(&countMutex);
    		availableThreads++;
    	pthread_mutex_unlock(&countMutex);
	}
	return NULL;
}