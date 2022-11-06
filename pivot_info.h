#ifndef PIVOT_INFO
#define PIVOT_INFO

struct pivotInfo; //Stores information about pivots
struct pivotInfo* pivotInfoConstructor(long arrSize); //Allocates memory for a pivotInfo, initialize the values properly and return the pointer to the pivotInfo
void pivotInfoDestructor(struct pivotInfo* info); //Free the memory
int pivotIndex(int i, int numOfWorkers); //Get the index of the i-th pivot in the sample array

#endif