#ifndef _2PL_H
#define _2PL_H
#include <vector>
using namespace std;
// 두가지 구조체가 만들어지는데 하나는 record의 구조체이고 또 하나는 thread의 구조체이다, 여기서 thread의 구조체가 특히 독특한데, 정확히 thread의 개수만큼 할당되어서 나중에 큐, 리스트에 추가 될 때 해당 thread의 이 구조체가 추가되게 된다.
struct rwQ{
	long tid;
	int curRec;		// 현재 thread의 구조체가 담겨있는 list가 어느 record에 달려 있는 지 확인해주는 변수
	int owning[2]; // 현재 thread의 read-lock과 첫번째 write-lock을 어느 record에 달려 있는 지 확인해주는 변수
	bool read; // 현재 thread의 구조체가 read-lock을 잡으려는 것인지 write-lock을 잡으려는 것인지 확인해주는 변수
	pthread_cond_t myCond;    // thread가 wait을 하게 될 시에 wait하는 조건변수
	rwQ * next;	// 리스트 내의 이 구조체 다음 구조체를 pointing하는 변수
	rwQ (){	// 생성자!, 몇몇 변수들을 초기화시킨다.
		owning[0] = -1; // id관련 변수들은 앞으로 모두 -1이 초기값이 된다.
		owning[1] = -1;
		next = NULL;
		myCond = PTHREAD_COND_INITIALIZER;
		curRec = -1;
	}
};
struct Record{
	rwQ * next;			// 현 record의 lock table을 담는 rwQ 포인터
	int ownerTid;		// 현 record에 lock을 걸고 있는 thread 의 id를 담고 있다
	bool ownerIsRead;	// 현 record에 lock을 걸고 있는 thread가 무슨 lock을 걸었는 지 확인해 주는 변수
	int readShares;		// 현 record에 만약 여러 개의 read-lock이 걸려 있을 경우 몇개의 read-lock이 현재 걸려 있는 지 확인해주는 변수
	int rid;			// record의 id
	int content;		// record가 담고 있는 실질적인 내용
	bool emptyRec;		// 현재 record에 lock을 걸고 있는 thread가 있는 지 없는 지
	Record(){ 	// 생성자 !, 몇몇 변수들을 초기화시킨다.
		emptyRec = true;
		content = 100;  // 초기 record가 담는 값은 100이다.
		next = NULL; 
		readShares = 0;
		ownerTid = -1;
	} 
};
vector<Record*> recVec;	// record의 vector 모든 record는 vector에 저장된다.
vector<rwQ*>thrRW;		// thread구조체의 vector, thread가 추가될 때마다 하나의 새로운 구조체가 생겨서 이 vector에 저장 된다.
int commit_id = 0;
int R, E;			// Record의 개수와, 실행 횟수 제한을 담고 있는 전역 변수
bool deadLockExist = false;		// deadlock 발생 시 그 결과를 알려 주기 위한 전역 boolean 변수
pthread_mutex_t globalMtx = PTHREAD_MUTEX_INITIALIZER;	// 몇 천개 몇만개의 thread가 생겨도 다 막아내어  정지된 state를 보게 해주는 global lock!!
#endif
