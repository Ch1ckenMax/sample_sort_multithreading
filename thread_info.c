#include <stdlib.h>
#include "thread_info.h"

struct threadInfo{
    //start and end index of the sub sequence corresponds to this thread
    long start;
    long end;

    //A pointer to an array for phase 4 (exchanging partitions)
    int* mergedSubSequence;
    long nextInsertPosition;
};

struct threadInfo* threadInfoConstructor(long start, long end){
    struct threadInfo* temp = (struct threadInfo*) malloc(sizeof(struct threadInfo));
    temp->start = start;
    temp->end = end;
    
    int* temp2 = (int*) malloc( (end - start)*sizeof(int));
    temp->mergedSubSequence = temp2;
    temp->nextInsertPosition = 0;

    return temp;
}

//Free the memory
void threadInfoDestructor(struct threadInfo* info){
    free(info->mergedSubSequence);
    free(info);
    return;
}