#include <stdlib.h>
#include "thread_info.h"

struct ThreadInfo* threadInfoConstructor(long start, long end){
    struct ThreadInfo* temp = (struct ThreadInfo*) malloc(sizeof(struct ThreadInfo));
    temp->start = start;
    temp->end = end;
    temp->mergedSubSequenceAllocated = 0;
    temp->mergedSubSequenceLength = 0;
    temp->nextInsertPosition = 0;

    return temp;
}

//Free the memory
void threadInfoDestructor(struct ThreadInfo* info){
    if(info->mergedSubSequenceAllocated == 1){
        free(info->mergedSubSequence);
    }
    free(info);
    return;
}