#ifndef PIVOT_INFO
#define PIVOT_INFO

//Stores information needed for pivots
struct PivotInfo{
    //Info for handling the samples from phase 1
    int* samples;
    int nextInsertPosition;
};

struct PivotInfo* pivotInfoConstructor(long arrSize); //Allocates memory for a pivotInfo, initialize the values properly and return the pointer to the pivotInfo
void pivotInfoDestructor(struct PivotInfo* info); //Free the memory
int pivotIndex(int i, int numOfWorkers); //Get the index of the i-th pivot in the sample array

#endif