#include "Queue.h"

// 큐 초기화 함수
void kInitializeQueue(QUEUE* pstQueue, void* pvQueueBuffer, int iMaxDataCount, int iDataSize){
	// 큐 최대 개수와 크기를 저장
	pstQueue->iMaxDataCount = iMaxDataCount;
	pstQueue->iDataSize = iDataSize;
	pstQueue->pvQueueArray = pvQueueBuffer;

	// 큐의 삽입 위치, 제거 위치 초기화
	pstQueue->iPutIndex = 0;
	pstQueue->iGetIndex = 0;
	pstQueue->bLastOperationPut = FALSE;
}

BOOL kIsQueueFull(const QUEUE* pstQueue){
	// get, put이 같고 전에 삽입했었다면 큐가 꽉찬것
	if((pstQueue->iGetIndex == pstQueue->iPutIndex) && (pstQueue->bLastOperationPut == TRUE))
		return TRUE;
	return FALSE;
}

BOOL kIsQueueEmpty(const QUEUE* pstQueue){
	// 전에 삽입한게 아니었다면
	if((pstQueue->iGetIndex == pstQueue->iPutIndex) && (pstQueue->bLastOperationPut == FALSE))
		return TRUE;
	return FALSE;
}

// 데이터 삽입
BOOL kPutQueue(QUEUE* pstQueue, const void* pvData){
	if(kIsQueueFull(pstQueue) == TRUE) return FALSE;

	// 삽입
	kMemCpy((char*)pstQueue->pvQueueArray + (pstQueue->iDataSize * pstQueue->iPutIndex), pvData, pstQueue->iDataSize);

	// 인덱스 변경
	pstQueue->iPutIndex = (pstQueue->iPutIndex + 1) % pstQueue->iMaxDataCount;
	pstQueue->bLastOperationPut = TRUE;
	return TRUE;
}

// 데이터 제거
BOOL kGetQueue(QUEUE* pstQueue, void* pvData){
	if(kIsQueueEmpty(pstQueue) == TRUE) return FALSE;

	// 제거
	kMemCpy(pvData, (char*) pstQueue->pvQueueArray + (pstQueue->iDataSize * pstQueue->iGetIndex), pstQueue->iDataSize);

	// 인덱스 변경
	pstQueue->iGetIndex = (pstQueue->iGetIndex + 1) % pstQueue->iMaxDataCount;
	pstQueue->bLastOperationPut = FALSE;
	return TRUE;
}

