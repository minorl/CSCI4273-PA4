#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

struct FunctionInfo
{
	void (*dFunc)(void*);
	void* arg;
};

class ThreadPool
{
public:
	ThreadPool(const size_t threadCount = 10);
	~ThreadPool();
	int dispatch_thread(void dispatch_function (void*),void *arg);
	bool thread_avail();

private:
	size_t numThreads;
	pthread_t* threads;
	pthread_mutex_t queueMutex;
	sem_t workSem;
	std::queue<struct FunctionInfo> workQueue;
	bool die;
	void* threadWork(void); 
	static void *work_helper(void *instance) {
		return ((ThreadPool*)instance)->threadWork();
	}
	size_t availableThreads;
	pthread_mutex_t countMutex;

};

#endif