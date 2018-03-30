#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <ctime>

using namespace std;
int length;

class StampedSnap {
	public:
		long stamp;
		int value;
		int* snap;

		StampedSnap (){
			stamp = 0;
			value = 0;
			snap = NULL;
		}
};

class WFSnapshot {
	public:
		StampedSnap* a_table;
		long updateNum;
		StampedSnap* collect(void);
		void update (int val, int tid);
		int * scan (void);

		WFSnapshot(int init){
			updateNum = 0;
			a_table = new StampedSnap[length];
			for(int i = 0 ; i < length; i ++){
				a_table[i].value = init;
			}
		}
};

StampedSnap* WFSnapshot::collect(){
	StampedSnap* copy = (StampedSnap*)calloc(length, sizeof(StampedSnap));
	for (int j = 0; j < length; j++)
		copy[j] = a_table[j];
	return copy;
}
void WFSnapshot::update(int val, int tid){
	int me = tid;
	int *tmp = scan();
	StampedSnap oldValue = a_table[me];
	a_table[me].stamp = oldValue.stamp+1;
	a_table[me].value = val;
	if(a_table[me].snap == NULL)a_table[me].snap = (int*)calloc(length, sizeof(int));
	for(int i = 0 ; i < length; i ++) a_table[me].snap[i] = tmp[i];
	updateNum ++;
	free(tmp);
}
int* WFSnapshot::scan(){
	StampedSnap* oldCopy;
	StampedSnap* newCopy;
	bool cFlag;
	int* result = (int*)calloc(length, sizeof(int));
	bool moved[length];
	for(int i = 0 ; i < length; i ++) moved[i] = false;
	oldCopy = collect();
	while (true) {
		cFlag = false;
		newCopy = collect();
		for (int j = 0; j < length; j++) {
			if (oldCopy[j].stamp != newCopy[j].stamp) {
				if (moved[j]) {
					for(int i = 0; i < length; i ++)result[i] = oldCopy[j].snap[i];
					free(oldCopy);
					free(newCopy);
					return result;
				} else {
					moved[j] = true;
					free(oldCopy);
					oldCopy = newCopy;
					cFlag = true;
					break;
				}
			}
		}
		if(cFlag)continue;
		for (int j = 0; j < length; j++)result[j] = newCopy[j].value;
		free(oldCopy);
		free(newCopy);
		return result;
	}
}
