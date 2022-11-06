#ifndef THREAD_INFO
#define THREAD_INFO

struct threadInfo; //Stores information about a worker thread
struct threadInfo* threadInfoConstructor(long start, long end); //Allocates memory for a threadInfo, initialize the values properly and return the pointer to the threadInfo
void threadInfoDestructor(struct threadInfo* info); //Free the memory

#endif