#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "Types.h"

// 구조체
#pragma pack (push, 1)

// 큐
typedef struct kQueueManagerStruct{
	int iDataSize; // 큐 데이터 하나의 크기
	int iMaxDataCount; // 최대 데이터 개수

	void* pvQueueArray; // 큐 포인터
	int iPutIndex; // tail
	int iGetIndex; // head

	BOOL bLastOperationPut; // 마지막 명령어가 삽입인지 여부, 이건 
}QUEUE;

#pragma pack (pop)

// 함수
void kInitializeQueue(QUEUE* pstQueue, void* pvQueueBuffer, int iMaxDataCount, int iDataSize);
BOOL kIsQueueFull(const QUEUE* pstQueue);
BOOL kIsQueueEmpty(const QUEUE* pstQueue);
BOOL kPutQueue(QUEUE* pstQueue, const void* pvData);
BOOL kGetQueue(QUEUE* pstQueue, void* pvData);

#endif /*__QUEUE_H__*/
