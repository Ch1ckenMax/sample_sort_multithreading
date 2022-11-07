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
pthread_cond_t workerThreadWaitCond;
pthread_mutex_t workerLock;
int phase1FinishedThreads = 0;
int phase2WaitingThreads = 0;
int phase3_1WaitingThreads = 0;
int phase3_2WaitingThreads = 0;
int phase4WaitingThreads = 0;

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

	//Initialization
	threads = (pthread_t*) malloc(numOfWorkers * sizeof(pthread_t));
	threadInfo = (struct ThreadInfo**) malloc(numOfWorkers * sizeof(struct ThreadInfo*));
	pthread_barrier_init(&phaseOneBarrier, NULL, numOfWorkers);

	pivotInfo = pivotInfoConstructor(numOfWorkers);

	pthread_mutex_init(&mainThreadWaitMutex,NULL);
	pthread_cond_init(&mainThreadWaitCond,NULL);
	pthread_cond_init(&workerThreadWaitCond,NULL);
	pthread_mutex_init(&workerLock,NULL);

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
	}


	//Phase 1: Worker threads started, wait for the worker thread to finish

	//..........Worker threads working on phase 1............//

		pthread_mutex_lock(&mainThreadWaitMutex);
		while(phase1FinishedThreads != numOfWorkers)
			pthread_cond_wait(&mainThreadWaitCond, &mainThreadWaitMutex);
		phase1FinishedThreads = 0;
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


	//Phase 2 finishes. Wake the worker threads up for phase 3
		pthread_mutex_lock(&mainThreadWaitMutex);
		while(phase2WaitingThreads != numOfWorkers) //Wait for the worker threads to go into waiting for condition variable workerThreadWaitCond
			pthread_cond_wait(&mainThreadWaitCond, &mainThreadWaitMutex);
		pthread_cond_broadcast(&workerThreadWaitCond); //All worker threads asleep. LADS WAKE THE FUCK UP!!!
		pthread_mutex_unlock(&mainThreadWaitMutex);

	//.........Worker threads working on the FIRST PART of phase 3 concurrently.............//

	//Wait until worker threads finishes FIRST PART of phase 3, then wake them up again to tell them to work on the second part of phase 3
		pthread_mutex_lock(&mainThreadWaitMutex);
		while(phase3_1WaitingThreads != numOfWorkers) //Wait for the worker threads to go into waiting for condition variable workerThreadWaitCond
			pthread_cond_wait(&mainThreadWaitCond, &mainThreadWaitMutex);
		pthread_cond_broadcast(&workerThreadWaitCond); //All worker threads asleep. LADS WAKE THE FUCK UP!!!
		pthread_mutex_unlock(&mainThreadWaitMutex);
	

	//.........Worker threads working on the SECOND PART of phase 3 concurrently.............//

	//Wait until worker threads finishes SECOND PART of phase 3, then wake them up again to tell them to work on phase 4
		pthread_mutex_lock(&mainThreadWaitMutex);
		while(phase3_2WaitingThreads != numOfWorkers) //Wait for the worker threads to go into waiting for condition variable workerThreadWaitCond
			pthread_cond_wait(&mainThreadWaitCond, &mainThreadWaitMutex);
		pthread_cond_broadcast(&workerThreadWaitCond); //All worker threads asleep. LADS WAKE THE FUCK UP!!!
		pthread_mutex_unlock(&mainThreadWaitMutex);

	//..........Worker threads working on the FIRST PART of phase 4 concurrently.............//

		pthread_mutex_lock(&mainThreadWaitMutex);
		while(phase4WaitingThreads != numOfWorkers) //Wait for the worker threads to go into waiting for condition variable workerThreadWaitCond
			pthread_cond_wait(&mainThreadWaitCond, &mainThreadWaitMutex);
		pthread_cond_broadcast(&workerThreadWaitCond); //All worker threads asleep. LADS WAKE THE FUCK UP!!!
		pthread_mutex_unlock(&mainThreadWaitMutex);

	//..........Worker threads working on the SECOND PART of phase 4 concurrently.............//

	//Worker threads' jobs are done
	for(int i = 0; i < numOfWorkers; i++){
		if(pthread_join(threads[i], NULL)){
			printf("Error occured when joining the worker thread %d to main thread!",i);
			exit(1);
		}
	}

	//Phase 5: main thread merge the subsequence back together
	long nextInsertPos = 0;
	for(int i = 0; i < numOfWorkers; i++){
		for(int j = 0; j < threadInfo[i]->mergedSubSequenceLength; j++){
			intarr[nextInsertPos] = threadInfo[i]->mergedSubSequence[j];
			nextInsertPos++;
		}
	}

	//Free memory
	for(int i = 0; i < numOfWorkers; i++){
		threadInfoDestructor(threadInfo[i]);
	}
	pthread_barrier_destroy(&phaseOneBarrier);
	pthread_mutex_destroy(&mainThreadWaitMutex);
	pthread_cond_destroy(&mainThreadWaitCond);
	pthread_cond_destroy(&workerThreadWaitCond);
	pthread_mutex_destroy(&workerLock);
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
void* workerThread(void* tInfo){
	struct ThreadInfo* info = (struct ThreadInfo*) tInfo;
	pthread_barrier_wait(&phaseOneBarrier); //Wait until all threads are created

	//Phase 1: sort its own subsequence
	qsort(&intarr[info->start], info->end - info->start, sizeof(unsigned int), compare);

	//Wake up main thread as phase 1 is finished
	pthread_mutex_lock(&mainThreadWaitMutex);
	pthread_cond_signal(&mainThreadWaitCond);
	phase1FinishedThreads++;
	pthread_mutex_unlock(&mainThreadWaitMutex);

	//.............Main thread Working on Phase 2..................//

	//Wait for main thread to finish phase 2
	pthread_mutex_lock(&mainThreadWaitMutex);
	pthread_cond_signal(&mainThreadWaitCond);
	phase2WaitingThreads++; //Indicates that this thread is ready to go to sleep and wake up together with other threads again in the cond_wait for workerThreadWaitCond
	pthread_cond_wait(&workerThreadWaitCond, &mainThreadWaitMutex); //Sleep and wait for main thread to say its good to go to phase 3
	pthread_mutex_unlock(&mainThreadWaitMutex);

	//Phase 3 (In my implementation for phase 3, I just find the number of elements for subsequence used in phase 4 instead of finding the index range of each partitions, then allocate heap memory to an array for storing the subsequence in phase 4)
	int belongingThread = 0;
	for(int i = info->start; i < info->end; i++){ //Trasverse through this thread's subSequence(retrieved from phase 1), and get the number of elements that belong to each thread's subSequence for phase 4
		//this element and the remaining elements of the array belongs to the rest partitions(the array is sorted implies all remaining elements are larger)
		//loop until the belongingThread is the largest partition
		while((belongingThread != numOfWorkers - 1) && (intarr[i] > pivotInfo->pivots[belongingThread])){ //short-circuit, so there will not be segmentation fault 
			belongingThread++;
		}
		pthread_mutex_lock(&workerLock);
		(threadInfo[belongingThread]->mergedSubSequenceLength)++;
		pthread_mutex_unlock(&workerLock);
	}

	//Signals that this worker thread has finished its FIRST part in phase 3. Tells the main thread about it and waits for main thread to tell this worker thread that it can proceed to second part of phase 3
	pthread_mutex_lock(&mainThreadWaitMutex);
	pthread_cond_signal(&mainThreadWaitCond);
	phase3_1WaitingThreads++;
	pthread_cond_wait(&workerThreadWaitCond, &mainThreadWaitMutex);
	pthread_mutex_unlock(&mainThreadWaitMutex);

	//second part in phase 3: allocating space for subsequences
	info->mergedSubSequence = (unsigned int*) malloc(info->mergedSubSequenceLength*sizeof(unsigned int));
	info->mergedSubSequenceAllocated = 1;

	pthread_mutex_lock(&mainThreadWaitMutex);
	pthread_cond_signal(&mainThreadWaitCond);
	phase3_2WaitingThreads++;
	pthread_cond_wait(&workerThreadWaitCond, &mainThreadWaitMutex);
	pthread_mutex_unlock(&mainThreadWaitMutex);

	//first part in phase 4: Put the partitions to the correct subSequence
	belongingThread = 0;
	for(int i = info->start; i < info->end; i++){
		while((belongingThread != numOfWorkers - 1) && (intarr[i] > pivotInfo->pivots[belongingThread])){ //short-circuit, so there will not be segmentation fault 
			belongingThread++;
		}
		pthread_mutex_lock(&workerLock);
		threadInfo[belongingThread]->mergedSubSequence[threadInfo[belongingThread]->nextInsertPosition] = intarr[i];
		threadInfo[belongingThread]->nextInsertPosition++;
		pthread_mutex_unlock(&workerLock);
	}

	pthread_mutex_lock(&mainThreadWaitMutex);
	pthread_cond_signal(&mainThreadWaitCond);
	phase4WaitingThreads++;
	pthread_cond_wait(&workerThreadWaitCond, &mainThreadWaitMutex);
	pthread_mutex_unlock(&mainThreadWaitMutex);

	////second part in phase 4: sort the sub sequences
	qsort(info->mergedSubSequence, info->nextInsertPosition, sizeof(unsigned int), compare);

}