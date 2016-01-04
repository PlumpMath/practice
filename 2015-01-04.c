/*
  Problem:
  Build a multithreaded circular work queue using processor atomics.

  Results: worked for 37 minutes
  I managed to get a little linear single producer multiple consumer work queue up and
  running in half an hour. I didn't get to making it a ring buffer. It could be cool to
  try and build on this to implement different concurrency patterns but I don't know if
  that is against the rules or not. I feel like 30 minutes isn't much time and it takes
  about 30 mins to type out all the boilerplate to do this. We'll see though, maybe it's
  just because I'm rusty. Felt overall like a good first day.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <libkern/OSAtomic.h> // OSX atomics header.

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))

typedef struct {
    char* work;
} WorkItem;

#define BUFFER_MAX_COUNT 10
#define NUM_WORK_THREADS 5

// Start with like a version that is serial, not circular for now.
typedef struct {
    WorkItem work_items[BUFFER_MAX_COUNT];
    int work_item_count;
    int work_item_max_count;

    volatile int next_item_to_process;
} WorkBuffer;

typedef struct {
    pthread_t handle;
    int logical_thread_index;

    WorkBuffer *buffer;
} ThreadInfo;

void* thread_process(void* data)
{
    ThreadInfo* info = (ThreadInfo*)data;
    printf("Launched %i\n", info->logical_thread_index);
    WorkBuffer *buffer = info->buffer;

    for (;;) {
        int next_item_to_process = buffer->next_item_to_process;
        if (next_item_to_process < buffer->work_item_count) {
            // try to claim it.
            if (OSAtomicCompareAndSwap32Barrier(next_item_to_process,
                                                next_item_to_process+1,
                                                &buffer->next_item_to_process)) {
                // process item.
                WorkItem* item = buffer->work_items + next_item_to_process;
                printf("Thread %i: %s\n", info->logical_thread_index, item->work);
            }
        }
    }
    
    return NULL;
}

int main()
{   
    char* work[] = {
        "One",
        "Two",
        "Three",
        "Four",
        "Five",
        "Six",
        "Seven",
        "Eight",
        "Nine",
        "Ten"
    };

    // Not a ring buffer yet.
    assert(ARRAY_COUNT(work) <= BUFFER_MAX_COUNT);

    WorkBuffer buffer;
    buffer.work_item_count = 0;
    buffer.work_item_max_count = BUFFER_MAX_COUNT;
    buffer.next_item_to_process = 0;

    ThreadInfo work_threads[NUM_WORK_THREADS];
    for (int thread_index = 0; thread_index < NUM_WORK_THREADS; thread_index++) {

        ThreadInfo *info = work_threads + thread_index;
        info->logical_thread_index = thread_index;
        info->buffer = &buffer;

        pthread_create(&info->handle, NULL, thread_process, (void*)info);
    }

    printf("Adding work\n");

    // Single producer so just need a memory barrior, not conflicts pushing.
    for (int work_index=0; work_index<ARRAY_COUNT(work); work_index++) {
        // Add work item
        assert(buffer.work_item_count < buffer.work_item_max_count);
        WorkItem* item = buffer.work_items + buffer.work_item_count;

        item->work = work[work_index];

        OSMemoryBarrier();

        buffer.work_item_count++;
    }

    // Give the threads a sec to do their work.
    sleep(5);
}

