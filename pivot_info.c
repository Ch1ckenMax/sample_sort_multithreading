#include <stdlib.h>
#include "pivot_info.h"

//Stores information needed for pivots
struct pivotInfo{
    //Info for handling the samples from phase 1
    int* samples;
    int nextInsertPosition;

};

//Allocates memory for a pivotInfo, initialize the values properly and return the pointer to the pivotInfo
struct pivotInfo* pivotInfoConstructor(long arrSize){
    struct pivotInfo* temp = (struct pivotInfo*) malloc(sizeof(struct pivotInfo));
    
    temp->samples = (int*) malloc(arrSize * sizeof(int));
    temp->nextInsertPosition = 0;

    return temp;
}

void pivotInfoDestructor(struct pivotInfo* info){
    free(info->samples);
    free(info);
}

int pivotIndex(int i, int numOfWorkers){
    return (i+1)*numOfWorkers + numOfWorkers/2 - 1;
}