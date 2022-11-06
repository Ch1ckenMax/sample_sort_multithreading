#include <stdlib.h>
#include "pivot_info.h"

//Allocates memory for a pivotInfo, initialize the values properly and return the pointer to the pivotInfo
struct PivotInfo* pivotInfoConstructor(long arrSize){
    struct PivotInfo* temp = (struct PivotInfo*) malloc(sizeof(struct PivotInfo));
    
    temp->samples = (int*) malloc(arrSize * sizeof(int));
    temp->nextInsertPosition = 0;

    return temp;
}

void pivotInfoDestructor(struct PivotInfo* info){
    free(info->samples);
    free(info);
}

int pivotIndex(int i, int numOfWorkers){
    return (i+1)*numOfWorkers + numOfWorkers/2 - 1;
}