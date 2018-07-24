#ifndef __LIST_H__
#define __LIST_H__

#include "Types.h"

// 구조체
#pragma pack(push, 1)

// 데이터 연결 자료구조, 데이터의 제일 앞부분에 위치 필수
typedef struct kListLinkStruct{
	void* pvNext;
	QWORD qwID;
} LISTLINK;

// 리스트 관리 자료구조
typedef struct kListManagerStruct{
	int iItemCount;
	void* pvHeader;
	void* pvTail;
} LIST;

#pragma pack(pop)

// 함수
void kInitializeList(LIST* pstList);
int kGetListCount(const LIST* pstList);
void kAddListToTail(LIST* pstList, void* pvItem);
void kAddListToHeader(LIST* pstList, void* pvItem);
void* kRemoveList(LIST* pstList, QWORD qwID);
void* kRemoveListFromHeader(LIST* pstList);
void* kRemoveListFromTail(LIST* pstList);
void* kFindList(const LIST* pstList, QWORD qwID);
void* kGetHeaderFromList(const LIST* pstList);
void* kGetTailFromList(const LIST* pstList);
void* kGetNextFromList(const LIST* pstList, void* pstCurrent);

#endif /*__LIST_H__*/
