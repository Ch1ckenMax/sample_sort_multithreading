#include <stdlib.h>
#include <pthread.h>
#include "thread_info.h"

struct ThreadInfo* threadInfoConstructor(long start, long end, int id){
    struct ThreadInfo* temp = (struct ThreadInfo*) malloc(sizeof(struct ThreadInfo));
    temp->id = id;
    temp->start = start;
    temp->end = end;
    temp->mergedSubSequenceAllocated = 0;
    temp->mergedSubSequenceLength = 0;
    temp->nextInsertPosition = 0;
    pthread_mutex_init(&(temp->mLock),NULL);

    return temp;
}

//Free the memory
void threadInfoDestructor(struct ThreadInfo* info){
    pthread_mutex_destroy(&(info->mLock));
    if(info->mergedSubSequenceAllocated == 1){
        free(info->mergedSubSequence);
    }
    free(info);
    return;
}