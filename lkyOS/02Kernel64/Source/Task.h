#ifndef __TASK_H__
#define __TASK_H__

#include "Types.h"
#include "List.h"

// 매크로
// SS, RSP, RFLAGS, CS, RIP와 SAVECONTEXT로 저장하는 레지스터 19개
#define TASK_REGISTERCOUNT (5 + 19)
#define TASK_REGISTERSIZE 8

// CONTEXT 자료구조의 레지스터 오프셋
#define TASK_GSOFFSET 0
#define TASK_FSOFFSET 1
#define TASK_ESOFFSET 2
#define TASK_DSOFFSET 3
#define TASK_R15OFFSET 4
#define TASK_R14OFFSET 5
#define TASK_R13OFFSET 6
#define TASK_R12OFFSET 7
#define TASK_R11OFFSET 8
#define TASK_R10OFFSET 9
#define TASK_R9OFFSET 10
#define TASK_R8OFFSET 11
#define TASK_RSIOFFSET 12
#define TASK_RDIOFFSET 13
#define TASK_RDXOFFSET 14
#define TASK_RCXOFFSET 15
#define TASK_RBXOFFSET 16
#define TASK_RAXOFFSET 17
#define TASK_RBPOFFSET 18
#define TASK_RIPOFFSET 19
#define TASK_CSOFFSET 20
#define TASK_RFLAGSOFFSET 21
#define TASK_RSPOFFSET 22
#define TASK_SSOFFSET 23

// 태스크 풀의 어드레스과 태스크 수
#define TASK_TCBPOOLADDRESS 0x800000
#define TASK_MAXCOUNT 1024

// 스택 풀과 스택 크기
#define TASK_STACKPOOLADDRESS (TASK_TCBPOOLADDRESS + sizeof(TCB) * TASK_MAXCOUNT)
#define TASK_STACKSIZE 8192

// 유효하지 않은 태스크 ID
#define TASK_INVALIDID 0xFFFFFFFFFFFFFFFF

// 태스크 프로세서 최대 점유 시간
#define TASK_PROCESSORTIME 5

// 준비 리스트 수
#define TASK_MAXREADYLISTCOUNT 5

// 태스크 우선순위
#define TASK_FLAGS_HIGHEST 0
#define TASK_FLAGS_HIGH 1
#define TASK_FLAGS_MEDUIM 2
#define TASK_FLAGS_LOW 3
#define TASK_FLAGS_LOWEST 4
#define TASK_FLAGS_WAIT 0xFF

// 태스크 플래그
#define TASK_FLAGS_ENDTASK 0x8000000000000000
#define TASK_FLAGS_IDLE 0x0800000000000000

// 함수 매크로
#define GETPRIORITY(x) ((x) & 0xFF)
#define SETPRIORITY(x, priority) ((x) = ((x) & 0xFFFFFFFFFFFFFF00) | (priority))
#define GETTCBOFFSET(x) ((x) & 0xFFFFFFFF)
// 구조체
#pragma pack(push, 1)

// 콘텍스트에 관련된 자료구조
typedef struct kContextStruct{
	QWORD vqRegister[TASK_REGISTERCOUNT];
} CONTEXT;

// 태스크 상태 관리 자료구조
typedef struct kTaskControlBlockStruct{
	// 다음 데이터의 위치와 ID, ID같은 경우 TCB 풀 관리 시만 쓰는데 각각의 구조체들은 바뀌지 않고 같은 위치에 항상 고정되어 있기 때문이다 반대로 위치같은 경우는 스케줄러 준비 리스트 관리시에만 사용한다, 리스트 속에서 다음 사용되어야 할 TCB를 가리키고 있게 된다
	LISTLINK stLink;

	// 플래그
	QWORD qwFlags;

	// 콘텍스트
	CONTEXT stContext;

	// 스택의 어드레스와 크기
	void* pvStackAddress;
	QWORD qwStackSize;
} TCB;

// TCB 풀의 상태를 관리하는 자료구조
typedef struct kTCBPoolManagerStruct{
	// 태스크 풀 정보
	TCB* pstStartAddress; // TCB의 시작 주소를 가리킴, 여기서 ID만큼 증가시키면 해당 TCB를 지정하게된다(배열 개념)
	int iMaxCount;
	int iUseCount;
	int iAllocatedCount; // 이 변수가 상위 32비트에 아이디와 합쳐지며 실제 xv6의 pid같은 역할을 하게됨	
}TCBPOOLMANAGER;

// 스케줄러 관리 자료구조
typedef struct kSchedulerStruct{
	TCB* pstRunningTask;

	int iProcessorTime;

	// 준비 리스트
	LIST vstReadyList[TASK_MAXREADYLISTCOUNT];

	// 대기 리스트
	LIST stWaitList;

	// 우선순위 별 실행 횟수
	int viExecuteCount[TASK_MAXREADYLISTCOUNT];

	// 프로세서 부하 계산용 변수
	QWORD qwProcessorLoad;

	// 유휴 태스크에서 사용한 프로세서 시간
	QWORD qwSpendProcessorTimeInIdleTask;
}SCHEDULER;

#pragma pack(pop)

// 함수
void kInitializeTCBPool(void);
TCB* kAllocateTCB(void);
void kFreeTCB(QWORD qwID);
TCB* kCreateTask(QWORD qwFlags, QWORD qwEntryPointAddress);
void kSetUpTask(TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void* pvStackAddress, QWORD qwStackSize);
void kInitializeScheduler(void);
void kSetRunningTask(TCB* pstTask);
TCB* kGetRunningTask(void);
TCB* kGetNextTaskToRun(void);
BOOL kAddTaskToReadyList(TCB* pstTask);
void kSchedule(void);
BOOL kScheduleInInterrupt(void);
void kDecreaseProcessorTime(void);
BOOL kIsProcessorTimeExpired(void);
TCB* kRemoveTaskFromReadyList(QWORD qwTaskID);
BOOL kChangePriority(QWORD qwID, BYTE bPriority);
BOOL kEndTask(QWORD qwID);
void kExitTask(void);
int kGetReadyTaskCount(void);
int kGetTaskCount(void);
TCB* kGetTCBInTCBPool(int iOffset);
BOOL kIsTaskExist(QWORD qwID);
QWORD kGetProcessorLoad(void);
void kIdleTask(void);
void kHaltProcessorByLoad(void);

#endif /*__TASK_H__*/
