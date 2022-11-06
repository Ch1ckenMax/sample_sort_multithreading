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

	//Initialize the threads
	threads = (pthread_t*) malloc(numOfWorkers * sizeof(pthread_t));
	threadInfo = (struct ThreadInfo**) malloc(numOfWorkers * sizeof(struct ThreadInfo*));
	pthread_barrier_init(&phaseOneBarrier, NULL, numOfWorkers);

	long tempStart = 0, tempEnd = 0; //Temp variables for creating threads

	for(int i = 0; i < numOfWorkers; i++){
		//Decide the index range of the sub sequence of i-th worker thread
		tempEnd = tempStart + size/numOfWorkers;
		if(i < size % numOfWorkers) //If the elements in the array is not divided evenly to the threads, add one more element to the subsequence that belongs to this thread if it is less than the remainder
			tempEnd++;
		threadInfo[i] = threadInfoConstructor(tempStart, tempEnd); //Initialize threadInfo
		tempStart = tempEnd;

		//Start the threads
		if(pthread_create(&threads[i], NULL, &workerThread, threadInfo[i])){
			printf("Error occured when creating worker threads!");
			exit(1);
		}

		//Debug
		printf("Thread %i is created!\n", i);
	}

	//Wait for the threads to finish Phase 1
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
	free(threadInfo);
	free(threads);

	//Debug
	for(int i = 0; i < size; i++){
		printf("%d: %d ",i , intarr[i]);
		printf("\n");
	}

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

	//Debug
	printf("Thread %jd starts to run!\n", info->start);


	//phase 1
	qsort(&intarr[info->start], info->end - info->start, sizeof(unsigned int), compare);
}