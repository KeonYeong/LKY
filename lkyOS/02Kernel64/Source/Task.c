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
	i = qwID & 0xFFFFFFFF;

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
	pvStackAddress = (void*)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * (pstTask->stLink.qwID & 0xFFFFFFFF)));

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
	kInitializeTCBPool();

	kInitializeList(&(gs_stScheduler.stReadyList));

	gs_stScheduler.pstRunningTask = kAllocateTCB(); // 현재 수행중인 태스크를 위한 공간을 할당받아놓는다
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
	if(kGetListCount(&(gs_stScheduler.stReadyList)) == 0) return NULL;

	return (TCB*)kRemoveListFromHeader(&(gs_stScheduler.stReadyList));
}

// 태스크를 준비 리스트에 삽입
void kAddTaskToReadyList(TCB* pstTask){
	kAddListToTail(&(gs_stScheduler.stReadyList), pstTask);
}

// 다른 태스크 찾아서 스케줄
void kSchedule(void){
	TCB* pstRunningTask, * pstNextTask;
	BOOL bPreviousFlag;

	if(kGetListCount(&(gs_stScheduler.stReadyList)) == 0) return;

	// 전환 중 인터럽트를 막기 위해 인터럽트 마스킹
	bPreviousFlag = kSetInterruptFlag(FALSE);

	pstNextTask = kGetNextTaskToRun(); // 다음 태스크 확인
	if(pstNextTask == NULL){
		kSetInterruptFlag(bPreviousFlag);
		return;
	}
	
	pstRunningTask = gs_stScheduler.pstRunningTask;
	kAddTaskToReadyList(pstRunningTask); // 현 수행 중 태스크는 큐에 넣어놓는다

	// 프로세서 사용 시간 업데이트
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

	// 다음 태스크를 수행
	gs_stScheduler.pstRunningTask = pstNextTask;
	kSwitchContext(&(pstRunningTask->stContext), &(pstNextTask->stContext));

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
	kMemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(CONTEXT));
	kAddTaskToReadyList(pstRunningTask);
	
	gs_stScheduler.pstRunningTask = pstNextTask;
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
