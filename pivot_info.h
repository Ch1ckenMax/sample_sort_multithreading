#ifndef PIVOT_INFO
#define PIVOT_INFO

//Stores information needed for pivots
struct PivotInfo{
    //Info for handling the samples from phase 1
    unsigned int* samples; //stores samples
    unsigned int* pivots; //stores pivots
    long nextInsertPosition;
    int samplesAllocated; //Indicates whether the samples array is allocated in the heap
};

struct PivotInfo* pivotInfoConstructor(long arrSize); //Allocates memory for a pivotInfo, initialize the values properly and return the pointer to the pivotInfo
void pivotInfoDestructor(struct PivotInfo* info); //Free the memory
long pivotIndex(long i, int numOfWorkers); //Get the index of the i-th pivot in the sample array

#endif