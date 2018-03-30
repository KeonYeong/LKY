#include <stdio.h>
#include <pthread.h>
#include <math.h>

#define NUM_THREAD  10

int thread_ret[NUM_THREAD];
int flag = 0;
int flag_number = 0;
int range_start;
int range_end;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
bool IsPrime(int n) {
	if (n < 2) {
		return false;
	}

	for (int i = 2; i <= sqrt(n); i++) {
		if (n % i == 0) {
			return false;
		}
	}
	return true;
}

void* ThreadFunc(void* arg) {
	while(1){
		pthread_mutex_lock(&mutex);
		while (flag == 0){
			printf("waitingthread");
			pthread_cond_wait(&cond, &mutex);
		}
		pthread_mutex_unlock(&mutex);
		printf("threadwakeup!!");
		long tid = (long)arg;

		// Split range for this thread
		int start = range_start + ((range_end - range_start) / NUM_THREAD) * tid;
		int end = range_start + ((range_end - range_start) / NUM_THREAD) * (tid+1);
		if (tid == NUM_THREAD - 1) {
			end = range_end + 1;
		}

		long cnt_prime = 0;
		for (int i = start; i < end; i++) {
			if (IsPrime(i)) {
				cnt_prime++;
			}
		}

		thread_ret[tid] = cnt_prime;
		pthread_mutex_lock(&mutex2);
		flag_number ++;
		if(flag_number == 10){
			pthread_cond_signal(&cond2);
		}
		pthread_mutex_unlock(&mutex2);
	}
}

int main(void) {
	pthread_t threads[NUM_THREAD];
	printf("wha???");
	for (long i = 0; i < NUM_THREAD; i++) {
		printf("what");
		if (pthread_create(&threads[i], 0, ThreadFunc, (void*)i) < 0) {
			printf("pthread_create error!\n");
			return 0;
		}
	}
	printf("???");
	while (1) {
		// Input range
		scanf("%d", &range_start);
		if (range_start == -1) {
			break;
		}
		scanf("%d", &range_end);
		pthread_mutex_lock(&mutex);
		printf("inside lock\n");
		flag = 1;
		pthread_cond_broadcast(&cond);
		printf("after broadcast\n");
		pthread_mutex_unlock(&mutex);
		flag = 0;
		// Create threads to work
		// Collect results
		pthread_mutex_lock(&mutex2);
		printf("main::now sleep\n");
		while(flag_number != 10){
			pthread_cond_wait(&cond2, &mutex2);
			printf("wake up sleep again");
		}
		pthread_mutex_unlock(&mutex2);
		flag_number = 0;
		int cnt_prime = 0;
		for (int i = 0; i < NUM_THREAD; i++) {
			cnt_prime += thread_ret[i];
		}
		printf("number of prime: %d\n", cnt_prime);
	}

	return 0;
}

