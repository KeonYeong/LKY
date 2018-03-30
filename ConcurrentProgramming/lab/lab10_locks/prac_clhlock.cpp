#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREAD 5
#define NUM_WORK 1000

int* CLHQueue[NUM_THREAD+1];
int* tail;
int cnt_global = 0;

void lock(int tid) {
    int* myNode = (int*)malloc(sizeof(int));
    CLHQueue[tid] = myNode;
    *CLHQueue[tid] = 1;
    int pred = __sync_lock_test_and_set(tail, tid);
    if(pred == 0)return;
    while(*CLHQueue[pred] == 1);{printf("w");}
    free(CLHQueue[pred]);

}

void unlock(int tid) {
    printf("unlocking");
    *CLHQueue[tid] = 0;
    __sync_synchronize();
}

void* Work(void* args){
    int tid  = (long long)args;
    for(int i = 0 ; i < NUM_WORK; i ++){
        lock(tid);
        cnt_global++;
        unlock(tid);
    }
}

int main(void){
    pthread_t threads[NUM_THREAD];
    long long i;
    *tail = 0;
    for (i = 0 ; i < NUM_THREAD; i ++){
        if (pthread_create(&threads[i], 0, Work, (void*)(i+1))<0){
            return 0;
        }
    }

    for (i = 0 ; i < NUM_THREAD; i++){
        pthread_join(threads[i], NULL);
    }
    printf("cnt_global: %d\n", cnt_global);

    return 0;
}

