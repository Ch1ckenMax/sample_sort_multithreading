#include <stdlib.h>
#include "pivot_info.h"

//Allocates memory for a pivotInfo, initialize the values properly and return the pointer to the pivotInfo
struct PivotInfo* pivotInfoConstructor(long numOfWorkers){
    struct PivotInfo* temp = (struct PivotInfo*) malloc(sizeof(struct PivotInfo));
    
    temp->samples = (unsigned int*) malloc(numOfWorkers*numOfWorkers * sizeof(unsigned int));
    temp->samplesAllocated = 1;
    temp->nextInsertPosition = 0;

    temp->pivots = (unsigned int*) malloc((numOfWorkers - 1) * sizeof(unsigned int));

    return temp;
}

void pivotInfoDestructor(struct PivotInfo* info){
    if(info->samplesAllocated == 1){ //Only free this if the samples is not freed beforehand
        free(info->samples);
    }
    free(info);
}

long pivotIndex(long i, int numOfWorkers){
    return (i+1)*numOfWorkers + numOfWorkers/2 - 1;
}