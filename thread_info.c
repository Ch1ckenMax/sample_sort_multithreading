#include <stdlib.h>
#include "thread_info.h"

struct ThreadInfo* threadInfoConstructor(long start, long end){
    struct ThreadInfo* temp = (struct ThreadInfo*) malloc(sizeof(struct ThreadInfo));
    temp->start = start;
    temp->end = end;
    
    unsigned int* temp2 = (unsigned int*) malloc( (end - start)*sizeof(unsigned int));
    temp->mergedSubSequence = temp2;
    temp->nextInsertPosition = 0;

    return temp;
}

//Free the memory
void threadInfoDestructor(struct ThreadInfo* info){
    free(info->mergedSubSequence);
    free(info);
    return;
}