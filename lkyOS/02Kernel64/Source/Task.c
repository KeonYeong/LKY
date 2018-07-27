#include "Task.h"
#include "Descriptor.h"

// 스케줄러 관련 자료구조
static SCHEDULER gs_stScheduler;
static TCBPOOLMANAGER gs_stTCBPoolManager;

//=========================
// 태스크와 태스크 풀 관련
//=========================

// 태스크 풀 초기화
void kInitializeTCBPool(void){
	int i;
	kMemSet(&(gs_stTCBPoolManager), 0, sizeof(gs_stTCBPoolManager));

	// 태스크 풀의 어드레스를 지정해 놓고, 그 사이즈 만큼 전부다 초기화한다 그후에 각 TCB에 전부 ID를 할당해준다
	gs_stTCBPoolManager.pstStartAddress = (TCB*) TASK_TCBPOOLADDRESS;
	kMemSet(TASK_TCBPOOLADDRESS, 0, sizeof(TCB) * TASK_MAXCOUNT);
	for(i = 0; i < TASK_MAXCOUNT; i ++) gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i; // stLink는 List의 연결 요소

	// TCB 최대 개수, 할당 횟수 초기화
	gs_stTCBPoolManager.iMaxCount = TASK_MAXCOUNT;
	gs_stTCBPoolManager.iAllocatedCount = 1;
}

// TCB 할당 받기
TCB* kAllocateTCB(void){
	TCB* pstEmptyTCB;
	int i;

	if(gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount) return NULL; // 만약 다 쓰고 있으면 그냥 리턴

	for(i = 0; i < gs_stTCBPoolManager.iMaxCount; i++){
		// ID가 비록 TCB 블럭들의 ID이기도 하지만 그것은 하위 32비트만 쓰는것이고 따라서 중복의 경우가 생기기 때문에 xv6에서 pid같은 역활을 하는것은 상위 32비트인 allocatedcount이다 따라서 그것을 비교한다. 또 해당 비트가 0이면 할당되지 않은 TCB라는 뜻
		if((gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID >> 32) == 0){
			pstEmptyTCB = &(gs_stTCBPoolManager.pstStartAddress[i]);
			break;
		}
	}

	// 상위 32비트를 다시 allocatedcount로 설정해준다
	pstEmptyTCB->stLink.qwID = ((QWORD)gs_stTCBPoolManager.iAllocatedCount << 32) | i;
	gs_stTCBPoolManager.iUseCount++;
	gs_stTCBPoolManager.iAllocatedCount++;
	if(gs_stTCBPoolManager.iAllocatedCount == 0) gs_stTCBPoolManager.iAllocatedCount = 1; // 만약을 대비해서 0이면 1로 바꿈
	return pstEmptyTCB;
}

// TCB 해제하기
void kFreeTCB(QWORD qwID){
	int i;

	// 태스크 하위 32비트는 인덱스 역할을 해준다
	i = GETTCBOFFSET(qwID);

	// TCB 초기화 후 ID 상위 비트 재설정
	kMemSet(&(gs_stTCBPoolManager.pstStartAddress[i].stContext), 0, sizeof(CONTEXT));
	gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;

	gs_stTCBPoolManager.iUseCount--;
}

// 태스크 생성하기
// 태스크 ID에 따라 스택 풀에 스택 자동할당
TCB* kCreateTask(QWORD qwFlags, QWORD qwEntryPointAddress){
	TCB* pstTask;
	void* pvStackAddress;

	pstTask = kAllocateTCB();
	if(pstTask == NULL) return NULL;

	// 태스크 ID로 스택 어드레스 잡아낸다
	pvStackAddress = (void*)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * GETTCBOFFSET(pstTask->stLink.qwID)));

	// TCB 설정 후 준비 리스트에 삽입, 스케줄링 대기
	kSetUpTask(pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);
	kAddTaskToReadyList(pstTask);
	return pstTask;
}

// 파라미터 사용 TCB 설정
void kSetUpTask(TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void* pvStackAddress, QWORD qwStackSize){
	// CONTEXT 자료구조를 0으로 우선 초기화
	kMemSet(pstTCB->stContext.vqRegister, 0, sizeof(pstTCB->stContext.vqRegister));
	
	// 스택 관련된 RSP, RBP를 설정(Descending이니까 Size를 더해서 지정
	pstTCB->stContext.vqRegister[TASK_RSPOFFSET] = (QWORD)pvStackAddress + qwStackSize;
	pstTCB->stContext.vqRegister[TASK_RBPOFFSET] = (QWORD)pvStackAddress + qwStackSize;

	// 세그먼트 셀렉터 설정
	pstTCB->stContext.vqRegister[TASK_CSOFFSET] = GDT_KERNELCODESEGMENT;
	pstTCB->stContext.vqRegister[TASK_DSOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_ESOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_FSOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_GSOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_SSOFFSET] = GDT_KERNELDATASEGMENT;

	// RIP 레지스터와 인터럽트플래그 설정
	pstTCB->stContext.vqRegister[TASK_RIPOFFSET] = qwEntryPointAddress;
	// RFLAGS의 IF 비트를 통해 인터럽트 플래그 설정
	pstTCB->stContext.vqRegister[TASK_RFLAGSOFFSET] |= 0x0200;

	// ID, 스택 플래그 저장
	pstTCB->pvStackAddress = pvStackAddress;
	pstTCB->qwStackSize = qwStackSize;
	pstTCB->qwFlags = qwFlags;
}

//=================
// 스케줄러 관련
//=================
// 스케줄러 초기화, TCB, init 태스크, 리스트 초기화
void kInitializeScheduler(void){
	int i;

	kInitializeTCBPool();

	// 준비 리스트 (5개)와 우선순위별 실행된 횟수 초기화, 대기리스트도 초기화
	for(i = 0; i < TASK_MAXREADYLISTCOUNT; i ++){
		kInitializeList(&(gs_stScheduler.vstReadyList[i]));
		gs_stScheduler.viExecuteCount[i] = 0;
	}
	kInitializeList(&(gs_stScheduler.stWaitList));

	gs_stScheduler.pstRunningTask = kAllocateTCB(); // 현재 수행중인 태스크를 위한 공간을 할당받아놓는다
	// 이 공간은 콘솔 셀을 위한 TCB이며, 따라서 우선순위를 먼저 설정한다
	gs_stScheduler.pstRunningTask->qwFlags = TASK_FLAGS_HIGHEST;

	// 프로세서 사용률 계산을 위한 변수 초기화
	gs_stScheduler.qwSpendProcessorTimeInIdleTask = 0;
	gs_stScheduler.qwProcessorLoad = 0;
}

// 현재 수행 중인 태스크 설정
void kSetRunningTask(TCB* pstTask){
	gs_stScheduler.pstRunningTask = pstTask;
}

// 현재 수행중인 태스크 반환
TCB* kGetRunningTask(void){
	return gs_stScheduler.pstRunningTask;
}

// 다음 수행할 태스크를 뽑아냄
TCB* kGetNextTaskToRun(void){
	TCB* pstTarget = NULL;
	int iTaskCount, i, j;

	// 맨 마지막 큐까지 전부 실행하고 나면 다시 맨 위것을 실행해야 하기 때문에 2번의 루프를 돌게 된다
	for(i = 0; i < 2; i ++){
		// 높은 큐에서 아래 큐까지 전부 보며 다음 태스크 찾는다
		for(j = 0; j < TASK_MAXREADYLISTCOUNT; j ++){
			iTaskCount = kGetListCount(&(gs_stScheduler.vstReadyList[j]));

			// 실행 횟수가 리스트 총 개수보다 많거나 같으면 아래 큐 선택!
			if(gs_stScheduler.viExecuteCount[j] < iTaskCount){
				pstTarget = (TCB*)kRemoveListFromHeader(&(gs_stScheduler.vstReadyList[j]));
				gs_stScheduler.viExecuteCount[j]++;
				break;
			}
			else gs_stScheduler.viExecuteCount[j] = 0; // 아래 큐로 넘어가기 전에 여기 실행 횟수는 다시 0으로 만들어준다
		}
		if(pstTarget != NULL) break; // 수행할 태스크가 존재 시 탈출
	}
	return pstTarget;
}

// 태스크를 준비 리스트에 삽입
BOOL kAddTaskToReadyList(TCB* pstTask){
	BYTE bPriority;

	bPriority = GETPRIORITY(pstTask->qwFlags);
	if(bPriority >= TASK_MAXREADYLISTCOUNT) return FALSE; // 유휴 리스트에 있는 경우이기에 바로 리턴
	kAddListToTail(&(gs_stScheduler.vstReadyList[bPriority]), pstTask);
	return TRUE;
}

// 준비 큐에서 다른 태스크 제거(우선순위 변경이나 종료시키고 싶을 때 큐에서 찾아서 제거하는 것)
TCB* kRemoveTaskFromReadyList(QWORD qwTaskID){
	TCB* pstTarget;
	BYTE bPriority;

	if(GETTCBOFFSET(qwTaskID) >= TASK_MAXCOUNT) return NULL; // 유효한 아이디만 취급

	// TCB 풀에서 아이디를 통해 바로 TCB를 찾아낸다
	pstTarget = &(gs_stTCBPoolManager.pstStartAddress[GETTCBOFFSET(qwTaskID)]);
	if(pstTarget->stLink.qwID != qwTaskID) return NULL; // 해당 TCB의 아이디가 만약 다르면 바로 리턴

	// 아이디를 찾아 맞는 큐에서 제거하기
	bPriority = GETPRIORITY(pstTarget->qwFlags);
	pstTarget = kRemoveList(&(gs_stScheduler.vstReadyList[bPriority]), qwTaskID);
	return pstTarget;
}

// 태스크 우선순위 변경함수
BOOL kChangePriority(QWORD qwTaskID, BYTE bPriority){
	TCB* pstTarget;

	if(bPriority > TASK_MAXREADYLISTCOUNT) return FALSE;

	// 현재 실행중인 태스크일 경우 우선순위만 변경한다, 나중에 스위치 시에 우선순위 보고 알아서 다른 큐에 넣기 때문
	pstTarget = gs_stScheduler.pstRunningTask;
	if(pstTarget->stLink.qwID == qwTaskID) SETPRIORITY(pstTarget->qwFlags, bPriority);
	// 실행중이 아니면 준비 리스트에서 찾아서 함
	else{
		pstTarget = kRemoveTaskFromReadyList(qwTaskID);
		// 준비 리스트에 들어가 있지 않은 태스크면 TCB 풀에서 걍 바로 찾아서 설정함
		if(pstTarget == NULL){
			pstTarget = kGetTCBInTCBPool(GETTCBOFFSET(qwTaskID));
			if(pstTarget != NULL) SETPRIORITY(pstTarget->qwFlags, bPriority);
		}
		else{
			SETPRIORITY(pstTarget->qwFlags, bPriority);
			kAddTaskToReadyList(pstTarget); // 맞는 우선순위에 다시 삽입
		}
	}
	return TRUE;
}

// 다른 태스크 찾아서 스케줄
void kSchedule(void){
	TCB* pstRunningTask, * pstNextTask;
	BOOL bPreviousFlag;

	if(kGetReadyTaskCount() < 1) return;

	// 전환 중 인터럽트를 막기 위해 인터럽트 마스킹
	bPreviousFlag = kSetInterruptFlag(FALSE);

	pstNextTask = kGetNextTaskToRun(); // 다음 태스크 확인
	if(pstNextTask == NULL){
		kSetInterruptFlag(bPreviousFlag);
		return;
	}

	pstRunningTask = gs_stScheduler.pstRunningTask;
	gs_stScheduler.pstRunningTask = pstNextTask;

	// 유휴 에서 전환된거면 해당 변수들 증가시킨다
	if((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME - gs_stScheduler.iProcessorTime;

	// 프로세서 사용 시간 업데이트
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

	// 태스크 종료 플래그가 설정되어 있으면 콘텍스트 저장은 안하고 대기리스트에만 삽입 후 콘텍스트 전환
	if(pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK){
		kAddListToTail(&(gs_stScheduler.stWaitList), pstRunningTask);
		kSwitchContext(NULL, &(pstNextTask->stContext));
	}
	else{
		// 다음 태스크를 수행
		kAddTaskToReadyList(pstRunningTask); // 현 수행 중 태스크는 큐에 넣어놓는다
		kSwitchContext(&(pstRunningTask->stContext), &(pstNextTask->stContext));
	}
	kSetInterruptFlag(bPreviousFlag);
}

// 인터럽트 발생 시 스케줄
BOOL kScheduleInInterrupt(void){
	TCB* pstRunningTask, * pstNextTask;
	char* pcContextAddress;

	pstNextTask = kGetNextTaskToRun();
	if(pstNextTask == NULL){
		return FALSE;
	}

	// 인터럽트 발생 시에는 어차피 콘텍스트들이 자동으로 IST에 저장되기 때문에 IST에 있는 값들을 전부 현재 태스크에 복사해놓고 IST에는 원하는 콘텍스트들을 복사해넣는다 IST는 0번이다 (타이머 인터럽트라서)
	pcContextAddress = (char*) IST_STARTADDRESS + IST_SIZE - sizeof(CONTEXT);

	pstRunningTask = gs_stScheduler.pstRunningTask;
	gs_stScheduler.pstRunningTask = pstNextTask;

	// 유휴 에서 전환된거면 해당 변수들 증가시킨다
	if((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;

	// 프로세서 사용 시간 업데이트
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

	// 태스크 종료 플래그가 설정되어 있으면 콘텍스트 저장은 안하고 대기리스트에만 삽입
	if(pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK) kAddListToTail(&(gs_stScheduler.stWaitList), pstRunningTask);
	else{
		// 아니면 IST 에서 현 태스크 콘텍스트에 복사하고 준비 리스트 삽입
		kMemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(CONTEXT));
		kAddTaskToReadyList(pstRunningTask);
	}

	kMemCpy(pcContextAddress, &(pstNextTask->stContext), sizeof(CONTEXT));

	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	return TRUE;
}

// 인터럽트 중 프로세서를 사용할 수 있는 시간 하나 감소
void kDecreaseProcessorTime(void){
	if(gs_stScheduler.iProcessorTime > 0)
		gs_stScheduler.iProcessorTime--;
}

// 프로세서 사용 가능한지 여부 (남은 시간 체크)
BOOL kIsProcessorTimeExpired(void){
	if(gs_stScheduler.iProcessorTime <= 0)
		return TRUE;
	return FALSE;
}

// 태스크 종료
BOOL kEndTask(QWORD qwTaskID){
	TCB* pstTarget;
	BYTE bPriority;

	// 현재 실행 중인 태스크일 경우 EndTask 비트 설정하고 schedule 호출
	pstTarget = gs_stScheduler.pstRunningTask;
	if(pstTarget->stLink.qwID == qwTaskID){
		pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
		SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
		kSchedule();

		while(1); // 전환되니까 아래 코드는 실행 안된다
	}
	// 실행 중이 아니면 준비 큐에서 찾아서 설정
	else{
		// 우선순위 변경 할때랑 비슷함
		pstTarget = kRemoveTaskFromReadyList(qwTaskID);
		if(pstTarget == NULL){
			pstTarget = kGetTCBInTCBPool(GETTCBOFFSET(qwTaskID));
			if(pstTarget != NULL){
				pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
				SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
			}
			return FALSE;
		}
		// 준비리스트에 존재 할 경우임
		pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
		SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
		kAddListToTail(&(gs_stScheduler.stWaitList), pstTarget);
	}
	return TRUE;
}

// 태스크 자신을 종료
void kExitTask(void){
	kEndTask(gs_stScheduler.pstRunningTask->stLink.qwID);
}

// 큐에 있는 모든 태스크 수 반환
int kGetReadyTaskCount(void){
	int iTotalCount = 0;
	int i;

	for(i = 0; i < TASK_MAXREADYLISTCOUNT; i ++)
		iTotalCount += kGetListCount(&(gs_stScheduler.vstReadyList[i]));

	return iTotalCount;
}

// 전체 태스크 수 반환
int kGetTaskCount(void){
	int iTotalCount;

	// 큐 태스크 수 + 대기 리스트 수 + 자기 자신
	iTotalCount = kGetReadyTaskCount();
	iTotalCount += kGetListCount(&(gs_stScheduler.stWaitList)) + 1;

	return iTotalCount;
}

TCB* kGetTCBInTCBPool(int iOffset){
	if((iOffset < -1) || (iOffset > TASK_MAXCOUNT)) return NULL;
	return &(gs_stTCBPoolManager.pstStartAddress[iOffset]);
}

// 태스크 존재 여부 반환
BOOL kIsTaskExist(QWORD qwID){
	TCB* pstTCB;

	pstTCB = kGetTCBInTCBPool(GETTCBOFFSET(qwID));
	if((pstTCB == NULL) || (pstTCB->stLink.qwID != qwID)) return FALSE;
	return TRUE;
}

// 프로세서 사용률 반환
QWORD kGetProcessorLoad(void){
	return gs_stScheduler.qwProcessorLoad;
}

// 유휴 태스크
void kIdleTask(void){
	TCB* pstTask;
	QWORD qwLastMeasureTickCount, qwLastSpendTickInIdleTask, qwCurrentMeasureTickCount, qwCurrentSpendTickInIdleTask;

	// 프로세서 사용량 계산을 위해 기준 정보 저장
	qwLastSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;
	qwLastMeasureTickCount = kGetTickCount();

	// 유휴 태스크는 항상 실행되어야 하기 때문에 무한루프를 돌려놓는다
	while(1){
		// 현 상태 저장
		qwCurrentMeasureTickCount = kGetTickCount();
		qwCurrentSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;

		// 사용량 계산
		// 100 - (유휴 태스크 사용시간) / (시스템 전체가 사용한 시간) * 100
		if(qwCurrentMeasureTickCount - qwLastMeasureTickCount == 0) gs_stScheduler.qwProcessorLoad = 0;
		else gs_stScheduler.qwProcessorLoad = 100 - (qwCurrentSpendTickInIdleTask - qwLastSpendTickInIdleTask) * 100 / (qwCurrentMeasureTickCount - qwLastMeasureTickCount);

		// 현 상태를 이전 쪽으로 보관
		qwLastMeasureTickCount = qwCurrentMeasureTickCount;
		qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;

		// 부하에 따라 쉬게 한다
		kHaltProcessorByLoad();

		// 대기 큐에 태스크가 존재 시 태스크 종료, 리소스 반환
		if(kGetListCount(&(gs_stScheduler.stWaitList)) >= 0){
			while(1){
				pstTask = kRemoveListFromHeader(&(gs_stScheduler.stWaitList));
				if(pstTask == NULL) break;
				kPrintf("IDLE: Task ID[0x%q] is completely ended\n", pstTask->stLink.qwID);
				kFreeTCB(pstTask->stLink.qwID);
			}
		}
		kSchedule();
	}
}

// 프로세서 쉬게하기
void kHaltProcessorByLoad(void){
	if(gs_stScheduler.qwProcessorLoad < 40){
		kHlt();
		kHlt();
		kHlt();
	}
	else if(gs_stScheduler.qwProcessorLoad < 80){
		kHlt();
		kHlt();
	}
	else if(gs_stScheduler.qwProcessorLoad < 95){
		kHlt();
	}
}
