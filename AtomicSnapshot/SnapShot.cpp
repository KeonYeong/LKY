#include "SnapShot.hpp"
WFSnapshot * sMemory;
void* ThreadFunc(void* arg){
	long tid = (long)arg;
	time_t curTime = time(NULL);
	while(time(NULL) <= curTime + 60){
		sMemory->update((int)tid, (int)tid);
	}
}

int main(int argc, char* argv[]){
	if(argc < 2){
		cout << "Need the number of Threads" << endl;
		return 0;
	}
	length = atoi(argv[1]);
	sMemory = new WFSnapshot(0);
	pthread_t threads[length];
	for (long i = 0 ; i < length; i ++){
		if(pthread_create(&threads[i], 0, ThreadFunc, (void*)i) < 0){
			cout << "pthread_create error" << endl;
			return 0;
		}
	}
	for (int i = 0 ; i < length; i ++){
		pthread_join(threads[i], NULL);
	}
	cout << "update : " << sMemory->updateNum << endl;
	return 0;
}
