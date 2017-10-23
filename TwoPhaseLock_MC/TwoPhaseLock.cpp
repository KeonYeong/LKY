#include <iostream>
#include <set>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <climits>
#include "2PL.hpp"		//2PL을 구현하기 위한 노드 구조체와, 레코드 구조체가 있다. 락 테이블 같은 경우는 별도로 잡지 않고
//레코드 구조체에 공간을 할당하여 구현하였다. 그 외에 여러 전역 변수 또한 정의되어 있다.

// 노드를 락테이블에 삽입하는 함수이다, 여기서 노드는 하나의 락을 대변하는 것이라 보면 되고 락테이블에 삽입함으로써 락을 걸겠다는 입장을 표명한 것이다.
void rwInsert(int rid, int tid){
	rwQ* tmp = recVec[rid]->next;
	if(tmp != NULL){
		while(tmp -> next)tmp = tmp->next;
		tmp->next = thrRW[tid];
	}
	else {
		recVec[rid]->next = thrRW[tid];
	}
}

// 데드락이 발생했을 시에 데드락을 처리해 주는 함수이다, 여기서는 데드락을 탐지한 노드가 본인의 모든 transaction을 포기함으로써 
// deadlock cycle을 삭제하고 후에 transaction을 다시 시도하게끔 구현했다.
void deadLockHandler(int rid, int tid){
	rwQ* tmp = recVec[rid]->next;
	// 우선 현재 레코드의 락테이블에 나 말고 다른 요소가 있는지 확인하여 있으면 거기에 넘겨준다.
	if(tmp != thrRW[tid]){
		while(tmp->next != thrRW[tid])tmp = tmp->next;
		tmp->next = NULL;
	}
	// 아닐 경우 다음 요소가 없다는 것이기에 락테이블은 비워지게 된다.
	else{
		recVec[rid]->next = NULL;
	}
	// 데드락을 탐지했다는 것은 반드시 write-lock에서 탐지 하게 될 수 밖에 없고, 그말은 즉 read-lock은 이미 어딘가에 잡아 두었다는 뜻이다
	// 여기서 owning[0]은 read-lock을 잡고 있는 레코드의 rid를 들고 있어서 그걸 토대로 레코드 점유 노드의 여러 변수들을 초기화 시켜주고 다음 노드를 깨운다.
	Record * tmpRec = recVec[thrRW[tid]->owning[0]];
	thrRW[tid]->owning[0] = -1;
	if(tmpRec->ownerTid == tid){
		tmpRec->emptyRec = true;
		tmpRec->ownerTid = -1;
		if(tmpRec->next)pthread_cond_signal(&tmpRec->next->myCond);
	}
	// write-lock은 경우가 다르다, 두 개가 있을 수도 있고 하나만 있을 수도 있지만 두번째 write-lock은 신경 쓸필요가 없다, 
	// 어차피 본인이 맨 뒤에 있기 때문이다. 따라서 첫번째  write-lock이 락테이블의 선두에 존재할 경우 역시 여러 변수들을 초기화 시키고 다음 노드를 깨우게 된다.
	if(thrRW[tid]->owning[1] >= 0){
		tmpRec = recVec[thrRW[tid]->owning[1]];
		tmpRec->emptyRec = true;
		tmpRec->ownerTid = -1;
		thrRW[tid]->owning[1] = -1;
		if(tmpRec->next)pthread_cond_signal(&tmpRec->next->myCond);
	}
	thrRW[tid]->curRec = -1;

}

void deadLockChecking(int rid, int tid){
	int tmp = tid;
	// 데드락이 발생했다는 것은 결국 Cycle이 생겼다는 것이다, 따라서 현재 추가했던 노드의 선두노드를 살펴보며 그 노드가 이어놓은 다른 노드를
	// 따라가고 다시 그 노드의 선두노드를 따라가고 하는 식으로 계속 간선을 따라가게 된다. 그러다 중간에 만약 같은 tid를 가진 노드가 선두에
	// 있는 경우 데드락이 발생될 수 있다고 판단하고 제거하는 함수를 호출하게 된다. 만약 데드락이 없다면 그냥 -1이라는 초기값이 나오게 되는데
	// 이 때 또한 루프를 강제로 탈출 하게 된다.
	while(recVec[thrRW[tmp]->curRec]->ownerTid != -1){
		tmp = recVec[thrRW[tmp]->curRec]->ownerTid;
		if(thrRW[tmp]->curRec == -1){
			//데드락 아님
			return;
		}
		if(tmp == tid){
			//데드락 발생!
			deadLockHandler(rid, tid);
			deadLockExist = true;
			return;
		}
	}
}

void readLock(int rid, int tid){
	// 해당 레코드가 비어있지 않으면서 해당 레코드의 선두노드가 write 노드이거나 이미 큐가 형성 되어있다면(이 말은 write-lock이 최소 한개 큐에 있다는 뜻),
	// 큐에 본인을 연결하고 wait을 호출한다.
	thrRW[tid]->read = true;
	if((!recVec[rid]->emptyRec && !recVec[rid]->ownerIsRead) || recVec[rid]->next){
		rwInsert(rid, tid);
		thrRW[tid]->curRec = rid;
		// write-lock의 경우 원래 이부분에서 deadLockChecking을 해주지만 read-lock 단계에서는 오직 하나의 노드만 추가된 것이기에 노드 추가로
		// 인한 데드락이 발생할 일이 절대 없다. 따라서 deadLockChecking을 스킵하게 되었다.
		do pthread_cond_wait(&thrRW[tid]->myCond, &globalMtx);
		//깨어나더라도 현재 레코드의 점유자가 있는지 확인하며 있다면 잘 못 일어난 것이기에 다시 잔다.
		while ((!recVec[rid]->emptyRec && !recVec[rid]->ownerIsRead) || (recVec[rid]->next && !recVec[rid]->next->read));
		//만약 자신이 실행될 차례이면서 본인 바로 뒤 스레드가 read-lock일 경우 바로 뒤 스레드도 같이 깨운다.
		if(thrRW[tid]->next && thrRW[tid]->next->read) pthread_cond_signal(&thrRW[tid]->next->myCond);
		// 여기로 넘어온 것은 이제 본인이 락을 잡을 차례라는 것이므로, 여기부터 함수 끝까지는 자기 자신을 링크(큐)에서 삭제하고 동시에
		// 레코드의 여러 변수들의 정보를 갱신하는 부분이다.
		recVec[rid]->next = thrRW[tid]->next;
		thrRW[tid]->next = NULL;
	}
	recVec[rid]->ownerTid = tid;
	thrRW[tid]->owning[0] = rid;
	thrRW[tid]->curRec = -1;
	recVec[rid]->ownerIsRead = thrRW[tid]->read;
	recVec[rid]->emptyRec = false;
}

void readUnLock(int rid, int tid){
	// owning[0] 여기서 어디에 read-lock을 잡았는 지 알 수 있기 위한 rid를 저장하고 있다.
	thrRW[tid]->owning[0] = -1;
	// 이 부분은 여러 Read-lock이 들어왔을 경우를 대비해서 조건처리가 되었다. 만약에 한번에 여러개의 Read-lock이 들어왔다면 모든
	// Read-lock들은 통과를 하게 되지만 unlock 시 깨워야 하는 노드는 하나밖에 없게 된다. 그렇기에 마지막으로 들어온 read-lock만 다음으로
	//존재하는 노드를 깨우게 되며 또한 record의 변수 역시 마지막으로 들어온 read-lock의 노드만 변경하게 된다.
	if(recVec[rid]->ownerTid == tid){
		recVec[rid]->emptyRec = true;
		recVec[rid]->ownerTid = -1;
		if(recVec[rid]->next)pthread_cond_signal(&recVec[rid]->next->myCond);
	}
}

void writeLock(int rid, int tid){
	thrRW[tid]->read = false;
	if(!recVec[rid]->emptyRec){
		rwInsert(rid, tid);
		thrRW[tid]->curRec = rid;
		// 이 윗부분 까지는 read-lock과 동일하다, 하지만 write-lock에선 이부분에서 deadlock을 탐지하게 된다. 그래서 deadLockChecking함수를
		// 호출하게 되고, 만약 데드락이 존재한다고 판단되면 deadLockChecking이 변경하는 deadLockExist라는 전역 boolean 변수의 값을 토대로
		// write-lock을 진행할지 아니면 포기할 지 정하게 된다.
		deadLockChecking(rid, tid);
		if(deadLockExist) return;
		// read-lock과 마찬가지로 계속해서 wait을 호출하게 된다, 다만 while 속의 조건문에 readShares가 0 초과일 경우가 추가 되었는데,
		// 이는 앞에 여러개의 read-lock이 존재할 경우를 대비해서이다, 여러개의 read-lock이 앞에 존재하지만 만약 마지막으로 들어간 read-lock이
		// 먼저 끝날 경우 본 write-lock을 깨우게 되는데 이 당시에 앞에 아직 여러개의 read-lock이 남아 있을 수 있기에 얼마나 남았는 지
		// 확인하여 만약 하나라도 남아있다면 바로 다시 wait을 호출하여 기다리게 된다.
		do pthread_cond_wait(&thrRW[tid]->myCond, &globalMtx);
		while (!recVec[rid]->emptyRec || recVec[rid]->readShares > 0);
		// 여기서 밑부분은 read-lock처럼 링크에서 본 노드를 삭제하고 현 record의 여러 변수들을 갱신해주는 작업이다.
		recVec[rid]->next = thrRW[tid]->next;
		thrRW[tid]->next = NULL;
	}
	recVec[rid]->ownerTid = tid;
	thrRW[tid]->curRec = -1;
	// owning[1]은 첫번째 write-lock이 어디에 걸려 있는 지를 표시해주는 변수이다, 따라서 만약 owning[1]이 아직 -1(초기값)이라면,
	// 첫번째 write-lock이라는 걸 알 수 있고 그에 따라 owning[1]을 갱신해준다. 이 모든 owning[0], owning[1]은 후에 deadlockChecking 시
	// deadlock이 탐지될 경우 해제해주기 위하여 위치를 표시해주는 변수들이다.
	if(thrRW[tid]->owning[1] == -1) thrRW[tid]->owning[1] = rid;
	recVec[rid]->ownerIsRead = thrRW[tid]->read;
	recVec[rid]->emptyRec = false;
}

void writeUnLock(int rid, int tid){
	//write-lock의 경우 unlock 시 read-lock와 달리  여러 개의 write-lock 같은 경우를 따질 필요가 없기에 바로 record의 변수들을 초기화하게 된다.
	// 그리고 다음 노드가 존재할 경우 바로 깨운다.
	recVec[rid]->emptyRec = true;
	recVec[rid]->ownerTid = -1;
	if(recVec[rid]->next)pthread_cond_signal(&recVec[rid]->next->myCond);
	// 현 unlock함수가 만약 첫번째 write-lock이 아닌 두번째 write-lock을 unlock할 경우 첫번째 write-lock의 정보를 담았던 owning[1]을 지워준다.
	if(thrRW[tid]->owning[1] != rid) thrRW[tid]->owning[1] = -1;
}

void * ThreadFunc(void * arg){
	long tid = (long)arg;
	int l, j, k, numL, numJ, numK;
	thrRW[tid]->tid = tid;
	ofstream commitStream("thread" + to_string(tid+1) + ".txt");
	while(1){
		//임의의 record를 선택하기 위해 먼저 랜덤으로 사용할 record의 index를 구한다.
		// 모든 index는 무조건 전 범위내에서 무작위로 뽑히며 만약 같은 숫자가 나올 경우 다시 뽑느다.
		l = rand()% R;
		do {
			j = rand()% R;
		} while(j == l);
		do {
			k = rand()% R;
		} while(k == l || k == j); 
		//첫번째 transaction
		pthread_mutex_lock(&globalMtx);
		readLock(l, tid);
		// read-lock을 성공적으로 잡았을 시에 readShares를 하나 올리는데 그  이유는 여러 개의 read-lock이 존재할 경우를 대비하기 위하여 만들었다,
		// 이 변수가 0 초과일 경우 항상 큐에 들어있는 바로 다음 write-lock(read-lock이 잡힌 record의 next는 항상 write-lock일 수밖에 없다)는
		// read-lock이 풀려도 readShares가 0이 될때까지 다시 wait 하게 된다.
		recVec[l]->readShares ++;
		pthread_mutex_unlock(&globalMtx);
		numL = recVec[l]->content;
		//두번째 transaction
		pthread_mutex_lock(&globalMtx);
		// readShares의 값을 여기서 하나 빼게 되는데 그 이유는 이 단계까지 스레드가 진행했다는 것은 이미 read-lock단계에서 read를 끝마쳤기 때문이다.
		recVec[l]->readShares--;
		// 만약 readShares의 값이 내가 뺌으로써 0이 되고, 또 해당 record의 원래 read-lock은 끝나서 비어있으며 또한 다음 노드가 존재할 시에,
		// 그 다음 노드 (write-lock)을 깨우게 된다.
		if(recVec[l]->readShares == 0 && recVec[l]->emptyRec && recVec[l]->next) pthread_cond_signal(&recVec[l]->next->myCond);
		//write-lock을 잡는다.
		writeLock(j, tid);
		// 만약 write-lock을 잡는 도중에 deadlock이 탐지되어서 deadLockExist가 true로 잡혀 있다면 그 즉시 deadLockExist는 초기화시키고 글로벌
		// 락을 풀면서 이번 transaction을 전부 포기하고 continue를 한다.
		if(deadLockExist) {
			deadLockExist = false;
			pthread_mutex_unlock(&globalMtx);
			continue;
		}
		pthread_mutex_unlock(&globalMtx);
		recVec[j]->content += numL + 1;
		numJ = recVec[j]->content;
		//세번째 transaction
		pthread_mutex_lock(&globalMtx);
		// write-lock을 잡고 그 후에 위에서와 마찬가지로 deadlock이 탐지되면 transaction을 포기하고 undo 하며 continue 한다.
		writeLock(k, tid);
		if(deadLockExist) {
			deadLockExist = false;
			recVec[j]->content -= (numL + 1);
			pthread_mutex_unlock(&globalMtx);
			continue;
		}
		pthread_mutex_unlock(&globalMtx);
		recVec[k]->content -= numL;
		numK = recVec[k]->content;
		//이 부분은 마지막 커밋 부분
		pthread_mutex_lock(&globalMtx);
		// 모든 lock을 여기서 풀어준다.
		writeUnLock(k, tid);
		writeUnLock(j, tid);
		readUnLock(l, tid);
		commit_id++;
		//커밋 아이디가 E를 초과했을 경우 Undo 후에 while문을 탈출하고 락을 해제한다.
		if (commit_id > E){
			recVec[j]->content -= (numL + 1);
			recVec[k]->content += numL;
			break;
		}
		//아닐 경우 정상적으로 thread#.txt파일에 commit log를 append한다.
		ofstream commitStream("thread" + to_string(tid+1) + ".txt", ios::app);
		if(commitStream.is_open()) commitStream << commit_id << ' ' << l << ' ' << j << ' ' << k << ' ' << numL << ' ' << numJ << ' ' << numK << endl;
		pthread_mutex_unlock(&globalMtx);
	}
	pthread_mutex_unlock(&globalMtx);
}
int main(int argc, char * argv[]){
	int N;
	srand((unsigned int)time(NULL));
	N = atoi(argv[1]);
	R = atoi(argv[2]);
	E = atoi(argv[3]);
	Record * tmpRec;
	rwQ * tmpRW;
	pthread_t threads[N];
	for(int i = 0 ; i < R; i ++){
		tmpRec = new Record();
		recVec.push_back(tmpRec);
	}
	for(long i = 0 ; i < N; i ++){
		tmpRW = new rwQ();
		thrRW.push_back(tmpRW);
		if(pthread_create(&threads[i], 0, ThreadFunc, (void*)i) < 0){
			cout << "pthread_create error!" << endl;
			return 0;
		}
	}
	for (int i = 0 ; i < N; i ++){
		pthread_join(threads[i], NULL);
	}
	// 모든 스레드가 종료되면 내 메모리는 소중하기에 모두 할당 해제해준다.
	for(vector<Record*>::iterator it = recVec.begin(); it != recVec.end(); it++) delete (*it);
	recVec.clear();
	for(vector<rwQ*>::iterator it = thrRW.begin(); it != thrRW.end(); it++) delete (*it);
	thrRW.clear();
	return 0;
}
