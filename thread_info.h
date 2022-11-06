#ifndef THREAD_INFO
#define THREAD_INFO

//Stores information about a worker thread
struct ThreadInfo{
    //start and end index of the sub sequence corresponds to this thread
    long start;
    long end;

    //A pointer to an array for phase 4 (exchanging partitions)
    int* mergedSubSequence;
    long nextInsertPosition;
};

struct ThreadInfo* threadInfoConstructor(long start, long end); //Allocates memory for a threadInfo, initialize the values properly and return the pointer to the threadInfo
void threadInfoDestructor(struct ThreadInfo* info); //Free the memory

#endif