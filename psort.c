// COMP3230 Programming Assignment Two
// The sequential version of the sorting using qsort

/*
# Filename: psort.c
# Student name and No.: Li Hoi Kit 3035745037
# Development platform: Windows WSL Ubuntu
# Remark:
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include "thread_info.h"
#include "pivot_info.h"

int checking(unsigned int *, long);
int compare(const void *, const void *);
void* workerThread(void*); //function for worker threads

// global variables
long size;  // size of the array
int numOfWorkers = 4; //Number of workers. By default is 4
unsigned int * intarr; // array of random integers
pthread_t* threads;
struct ThreadInfo** threadInfo;
struct PivotInfo* pivotInfo;
pthread_barrier_t phaseOneBarrier;

//Condition Variables and mutex locks
pthread_mutex_t mainThreadWaitMutex;
pthread_cond_t mainThreadWaitCond;
int phase1FinishedThreads = 0;
int phase2WaitingThreads = 0;
pthread_mutex_t workerThreadWaitMutex;
pthread_cond_t workerThreadWaitCond;

int main (int argc, char **argv)
{
	long i, j;
	struct timeval start, end;

	if ((argc != 2 && argc != 3))
	{
		printf("Usage: psort <number> [<number_of_workers>]\n");
		exit(0);
	}

	//Validate number_of_workers input
	if(argc == 3){
		numOfWorkers = atol(argv[2]);
		if(numOfWorkers < 2){
			printf("Number of workers must be greater than 1\n");
			exit(0);
		}
	}

	//Debug
	printf("Number of workers: %d\n", numOfWorkers);

	size = atol(argv[1]);
	intarr = (unsigned int *)malloc(size*sizeof(unsigned int));
	if (intarr == NULL) {perror("malloc"); exit(0); }
	
	// set the random seed for generating a fixed random
	// sequence across different runs
  char * env = getenv("RANNUM");  //get the env variable
  if (!env)                       //if not exists
		srandom(3230);
	else
		srandom(atol(env));
	
	for (i=0; i<size; i++) {
		intarr[i] = random();
	}
	
	// measure the start time
	gettimeofday(&start, NULL);
	
	// just call the qsort library
	// replace qsort by your parallel sorting algorithm using pthread
	//qsort(intarr, size, sizeof(unsigned int), compare);

	//Initialization
	threads = (pthread_t*) malloc(numOfWorkers * sizeof(pthread_t));
	threadInfo = (struct ThreadInfo**) malloc(numOfWorkers * sizeof(struct ThreadInfo*));
	pthread_barrier_init(&phaseOneBarrier, NULL, numOfWorkers);

	pivotInfo = pivotInfoConstructor(numOfWorkers);

	pthread_mutex_init(&mainThreadWaitMutex,NULL);
	pthread_cond_init(&mainThreadWaitCond,NULL);
	pthread_mutex_init(&workerThreadWaitMutex,NULL);
	pthread_cond_init(&workerThreadWaitCond,NULL);

	long tempStart = 0, tempEnd = 0; //Temp variables for creating threads

	//Worker thread creation
	for(int i = 0; i < numOfWorkers; i++){
		//Decide the index range of the sub sequence of i-th worker thread
		tempEnd = tempStart + size/numOfWorkers;
		if(i < size % numOfWorkers) //If the elements in the array is not divided evenly to the threads, add one more element to the subsequence that belongs to this thread if it is less than the remainder
			tempEnd++;
		threadInfo[i] = threadInfoConstructor(tempStart, tempEnd); //Initialize threadInfo
		tempStart = tempEnd;

		//Create the threads
		if(pthread_create(&threads[i], NULL, &workerThread, threadInfo[i])){
			printf("Error occured when creating worker threads!");
			exit(1);
		}

		//Debug
		printf("Thread %i is created!\n", i);
	}

	//Phase 1: Worker threads started, wait for the worker thread to finish
	pthread_mutex_lock(&mainThreadWaitMutex);
		while(phase1FinishedThreads != numOfWorkers){
			pthread_cond_wait(&mainThreadWaitCond, &mainThreadWaitMutex);
		}
		printf("main thread ft%d\n",phase1FinishedThreads);
	pthread_mutex_unlock(&mainThreadWaitMutex);

	//Phase 2
		//Gather samples
		for(int i = 0; i < numOfWorkers; i++){
			for(int j = 0; j < numOfWorkers; j++){ //Choose p samples from the sorted subsequence of i-th worker thread and store it to the samples array
				long sampleElementIndex = threadInfo[i]->start + j*size/(numOfWorkers*numOfWorkers); //Get the index of the element in the subsequence
				pivotInfo->samples[pivotInfo->nextInsertPosition] = intarr[sampleElementIndex]; //Insert the sample from the correct position
				(pivotInfo->nextInsertPosition)++; //Insert to the next position in the next loop
			}
		}
		//Sort the samples
		qsort(pivotInfo->samples, pivotInfo->nextInsertPosition, sizeof(unsigned int), compare);
		//Get the pivots and store it to the pivot array
		for(long i = 0; i < numOfWorkers - 1; i++){
			pivotInfo->pivots[i] = pivotInfo->samples[pivotIndex(i, numOfWorkers)];
		}
		//Free samples as we don't need them anymore
		free(pivotInfo->samples);
		pivotInfo->samplesAllocated = 0;



	//Debug
	//for(int i = 0; i < size; i++){
	//	printf("%d: %d, ",i , intarr[i]);
	//	printf("\n");
	//}

	printf("Pivots \n");
	for(long i = 0; i < numOfWorkers - 1; i++){
		
		printf("%jd: %d, ",i , pivotInfo->pivots[i]);
		printf("\n");
	}
	printf("\n");

	//Phase 2 finishes. Wake the worker threads up for phase 3
	pthread_mutex_lock(&mainThreadWaitMutex);
		while(phase2WaitingThreads != numOfWorkers){ //Wait for the worker threads to go into waiting for condition variable workerThreadWaitCond
			pthread_cond_wait(&mainThreadWaitCond, &mainThreadWaitMutex);
		}
		pthread_cond_broadcast(&workerThreadWaitCond); //All worker threads asleep. LADS WAKE THE FUCK UP!!!
	pthread_mutex_unlock(&mainThreadWaitMutex);

	printf("main thread start go to fucking sleep i suppose?\n");


	//Shits done
	for(int i = 0; i < numOfWorkers; i++){
		if(pthread_join(threads[i], NULL)){
			printf("Error occured when joining the worker thread %d to main thread!",i);
			exit(1);
		}
	}

	//Free memory
	for(int i = 0; i < numOfWorkers; i++){
		threadInfoDestructor(threadInfo[i]);
	}
	pthread_barrier_destroy(&phaseOneBarrier);
	pthread_mutex_destroy(&mainThreadWaitMutex);
	pthread_cond_destroy(&mainThreadWaitCond);
	pthread_mutex_destroy(&workerThreadWaitMutex);
	pthread_cond_destroy(&workerThreadWaitCond);
	pivotInfoDestructor(pivotInfo);
	free(threadInfo);
	free(threads);


	// measure the end time
	gettimeofday(&end, NULL);
	
	if (!checking(intarr, size)) {
		printf("The array is not in sorted order!!\n");
	}
	
	printf("Total elapsed time: %.4f s\n", (end.tv_sec - start.tv_sec)*1.0 + (end.tv_usec - start.tv_usec)/1000000.0);
	  
	free(intarr);
	return 0;
}

int compare(const void * a, const void * b) {
	return (*(unsigned int *)a>*(unsigned int *)b) ? 1 : ((*(unsigned int *)a==*(unsigned int *)b) ? 0 : -1);
}

int checking(unsigned int * list, long size) {
	long i;
	printf("First : %d\n", list[0]);
	printf("At 25%%: %d\n", list[size/4]);
	printf("At 50%%: %d\n", list[size/2]);
	printf("At 75%%: %d\n", list[3*size/4]);
	printf("Last  : %d\n", list[size-1]);
	for (i=0; i<size-1; i++) {
		if (list[i] > list[i+1]) {
		  return 0;
		}
	}
	return 1;
}

//Function for workerThread. 
void* workerThread(void* threadInfo){
	struct ThreadInfo* info = (struct ThreadInfo*) threadInfo;
	pthread_barrier_wait(&phaseOneBarrier); //Wait until all threads are created

	printf("Thread %jd starts to run!\n", info->start);//Debug

	//Phase 1: sort its own subsequence
	qsort(&intarr[info->start], info->end - info->start, sizeof(unsigned int), compare);

	//Wake up main thread as phase 1 is finished
	pthread_mutex_lock(&mainThreadWaitMutex);
		pthread_cond_signal(&mainThreadWaitCond);
		phase1FinishedThreads++;
		printf("Thread %jd haha!%d\n", info->start, phase1FinishedThreads);
	pthread_mutex_unlock(&mainThreadWaitMutex);

	//Wait for main thread to finish phase 2
	pthread_mutex_lock(&mainThreadWaitMutex);
		pthread_cond_signal(&mainThreadWaitCond);
		phase2WaitingThreads++; //Indicates that this thread is ready to go to sleep and wake up together with other threads again in the cond_wait for workerThreadWaitCond
		printf("Thread %jd dada!%d\n", info->start, phase2WaitingThreads);
		pthread_cond_wait(&workerThreadWaitCond, &mainThreadWaitMutex); //Sleep and wait for main thread to say its good to go to phase 3
	pthread_mutex_unlock(&mainThreadWaitMutex);

	//Phase 3
	

	printf("Thread %jd is now out of phase 2!\n", info->start);

}