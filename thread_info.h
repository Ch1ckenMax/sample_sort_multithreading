#ifndef THREAD_INFO
#define THREAD_INFO

//Stores information about a worker thread
struct ThreadInfo{
    //start and end index of the sub sequence corresponds to this thread
    long start;
    long end;

    unsigned int* mergedSubSequence; //A pointer to an array for phase 4 (collected partitions)
    int mergedSubSequenceAllocated; //Indicates if any memory from heap is allocated to the mergedSubSequence pointer
    long mergedSubSequenceLength;
    long nextInsertPosition;
};

struct ThreadInfo* threadInfoConstructor(long start, long end); //Allocates memory for a threadInfo, initialize the values properly and return the pointer to the threadInfo
void threadInfoDestructor(struct ThreadInfo* info); //Free the memory

#endif