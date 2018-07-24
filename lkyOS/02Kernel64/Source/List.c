#include "List.h"

// 리스트 초기화
void kInitializeList(LIST* pstList){
	pstList->iItemCount = 0;
	pstList->pvHeader = NULL;
	pstList->pvTail = NULL;
}

// 리스트에 포함된 아이템 수 반환
int kGetListCount(const LIST* pstList){
	return pstList->iItemCount;
}

// 리스트에 끝 부분에 데이터를 더함
void kAddListToTail(LIST* pstList, void* pvItem){
	LISTLINK* pstLink;

	// 다음 데이터의 어드레스는 NULL임
	pstLink = (LISTLINK*)pvItem;
	pstLink->pvNext = NULL;

	// 리스트가 비면 head tail을 본 데이터로 설정
	if(pstList->pvHeader == NULL){
		pstList->pvHeader = pvItem;
		pstList->pvTail = pvItem;
		pstList->iItemCount = 1;
		return;
	}
	
	// 마지막 데이터 위치를 구해서 이 추가한 애들 넣어줌
	pstLink = (LISTLINK*) pstList->pvTail;
	pstLink->pvNext = pvItem;

	// tail 변경
	pstList->pvTail = pvItem;
	pstList->iItemCount++;
}

// 리스트 첫 부분에 데이터 더함
void kAddListToHeader(LIST* pstList, void* pvItem){
	LISTLINK* pstLink;

	// 다음 데이터의 어드레스는 현 header임
	pstLink = (LISTLINK*)pvItem;
	pstLink->pvNext = pstList->pvHeader;

	// 리스트가 비면 head tail을 본 데이터로 설정
	if(pstList->pvHeader == NULL){
		pstList->pvHeader = pvItem;
		pstList->pvTail = pvItem;
		pstList->iItemCount = 1;
		return;
	}
	
	//header 변경
	pstList->pvHeader = pvItem;
	pstList->iItemCount++;
}

// 리스트에서 데이터 제거하고 포인터 반환
void* kRemoveList(LIST* pstList, QWORD qwID){
	LISTLINK* pstLink;
	LISTLINK* pstPreviousLink;

	// 일단 검색
	pstPreviousLink = (LISTLINK*) pstList->pvHeader;
	for(pstLink = pstPreviousLink; pstLink != NULL; pstLink = pstLink->pvNext){
		if(pstLink->qwID == qwID){ // 일치하는 ID를 가진 애가 있으면 제거함
			if((pstLink == pstList->pvHeader) && (pstLink == pstList->pvTail)){ // 데이터 하나면 리스트 초기화
				pstList->pvHeader = NULL;
				pstList->pvTail = NULL;
			}

			// 만약 리스트의 첫번째면 헤드 바꿔줌
			else if(pstLink == pstList->pvHeader) pstList->pvHeader = pstLink->pvNext;
			// 테일이면 테일을 전 아이템으로 바꿔줌
			else if(pstLink == pstList->pvTail) pstList->pvTail = pstPreviousLink;
			else pstPreviousLink->pvNext = pstLink->pvNext;

			pstList->iItemCount--;
			return pstLink;
		}
		pstPreviousLink = pstLink;
	}
	return NULL;
}

// 리스트의 첫번째 데이터 제거 후 반환
void* kRemoveListFromHeader(LIST* pstList){
	LISTLINK* pstLink;

	if(pstList->iItemCount == 0) return NULL;

	pstLink = (LISTLINK*) pstList->pvHeader;
	return kRemoveList(pstList, pstLink->qwID);
}

// 리스트의 마지막 데이터 제거 후 반환
void* kRemoveListFromTail(LIST* pstList){
	LISTLINK* pstLink;

	if(pstList->iItemCount == 0) return NULL;

	// 테일 제거 후 반환
	pstLink = (LISTLINK*)pstList->pvTail;
	return kRemoveList(pstList, pstLink->qwID);
}

// 리스트에서 아이템 찾기
void* kFindList(const LIST* pstList, QWORD qwID){
	LISTLINK* pstLink;
	
	for(pstLink = (LISTLINK*) pstList->pvHeader; pstLink != NULL; pstLink = pstLink->pvNext)
		if(pstLink->qwID == qwID) return pstLink;

	return NULL;
}

// 리스트의 헤더를 반환
void* kGetHeaderFromList(const LIST* pstList){
	return pstList->pvHeader;
}

// 리스트의 테일 반환
void* kGetTailFromList(const LIST* pstList){
	return pstList->pvTail;
}

// 현 아이템의 다음 아이템 반환
void* kGetNextFromList(const LIST* pstList, void* pstCurrent){
	LISTLINK* pstLink;
	pstLink = (LISTLINK*) pstCurrent;
	return pstLink->pvNext;
}
