#include "Descriptor.h"
#include "Utility.h"

// GDT===========================================================
// GDT 테이블 초기화
void kInitializeGDTTableAndTSS(void){
	GDTR* pstGDTR;
	GDTENTRY8* pstEntry;
	TSSSEGMENT* pstTSS;
	int i;

	// GDTR 설정
	pstGDTR = (GDTR*) GDTR_STARTADDRESS; // 0x142000은 1MB의 영역에서 264KB를 전 장에서 페이지 테이블로 썼기 때문에 그 후부터 GDTR로 쓴다, 그 뒤에 GDT, TSS가 온다
	pstEntry = (GDTENTRY8*) (GDTR_STARTADDRESS + sizeof(GDTR)); // GDT 위치
	pstGDTR->wLimit = GDT_TABLESIZE - 1; // 리미트는 3개의 GDT(널 디스크립터, 코드, 데이터) 그리고 1개의 TSS
	pstGDTR->qwBaseAddress = (QWORD) pstEntry;

	// TSS 설정
	pstTSS = (TSSSEGMENT*) ((QWORD)pstEntry + GDT_TABLESIZE);

	// NULL, Code, Data, TSS 4개의 세그먼트 생성
	// kSetGDTEntry8 는 세그먼트 디스크립터를 설정하는 함수
	kSetGDTEntry8(&(pstEntry[0]), 0, 0, 0, 0, 0); // 널 디스크립터
	kSetGDTEntry8(&(pstEntry[1]), 0, 0xFFFFF, GDT_FLAGS_UPPER_CODE, GDT_FLAGS_LOWER_KERNELCODE, GDT_TYPE_CODE); // 코드 디스크립터
	kSetGDTEntry8(&(pstEntry[2]), 0, 0xFFFFF, GDT_FLAGS_UPPER_DATA, GDT_FLAGS_LOWER_KERNELDATA, GDT_TYPE_DATA); // 데이터 디스크립터
	kSetGDTEntry16((GDTENTRY16*)&(pstEntry[3]), (QWORD)pstTSS, sizeof(TSSSEGMENT) - 1, GDT_FLAGS_UPPER_TSS, GDT_FLAGS_LOWER_TSS, GDT_TYPE_TSS); // TSS 디스크립터

	// TSS 초기화(GDT 다음 영역을 사용)
	kInitializeTSSSegment(pstTSS);
}

// GDT 엔트리에 값을 설정하는 것 (8바이트, 코드 / 데이터)
void kSetGDTEntry8(GDTENTRY8* pstEntry, DWORD dwBaseAddress, DWORD dwLimit, BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType){
	pstEntry->wLowerLimit = dwLimit & 0xFFFF;
	pstEntry->wLowerBaseAddress = dwBaseAddress & 0xFFFF;
	pstEntry->bUpperBaseAddress1 = (dwBaseAddress >> 16) & 0xFF;
	pstEntry->bTypeAndLowerFlag = bLowerFlags | bType;
	pstEntry->bUpperLimitAndUpperFlag = ((dwLimit >> 16) & 0xFF) | bUpperFlags;
	pstEntry->bUpperBaseAddress2 = (dwBaseAddress >> 24) & 0xFF;
}

// GDT 엔트리에 값을 설정하는 것 (16바이트 TSS)
void kSetGDTEntry16(GDTENTRY16* pstEntry, QWORD qwBaseAddress, DWORD dwLimit, BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType){
	pstEntry->wLowerLimit = dwLimit & 0xFFFF;
	pstEntry->wLowerBaseAddress = qwBaseAddress & 0xFFFF;
	pstEntry->bMiddleBaseAddress1 = (qwBaseAddress >> 16) & 0xFF;
	pstEntry->bTypeAndLowerFlag = bLowerFlags | bType;
	pstEntry->bUpperLimitAndUpperFlag = ((dwLimit >> 16) & 0xFF) | bUpperFlags;
	pstEntry->bMiddleBaseAddress2 = (qwBaseAddress >> 24) & 0xFF;
	pstEntry->dwUpperBaseAddress = qwBaseAddress >> 32;
	pstEntry->dwReserved = 0;
}

void kInitializeTSSSegment(TSSSEGMENT* pstTSS){
	kMemSet(pstTSS, 0, sizeof(TSSSEGMENT));
	pstTSS->qwIST[0] = IST_STARTADDRESS + IST_SIZE;
	pstTSS->wIOMapBaseAddress = 0xFFFF;
}

// IDT===========================================================
// IDT 게이트 디스크립터 생성 코드 (kSetGDTEntry 같은 것)
void kSetIDTEntry(IDTENTRY* pstEntry, void* pvHandler, WORD wSelector, BYTE bIST, BYTE bFlags, BYTE bType){
	pstEntry->wLowerBaseAddress = (QWORD) pvHandler & 0xFFFF;
	pstEntry->wSegmentSelector = wSelector;
	pstEntry->bIST = bIST & 0x3;
	pstEntry->bTypeAndFlags = bType | bFlags;
	pstEntry->wMiddleBaseAddress = ((QWORD) pvHandler >> 16) & 0xFFFF;
	pstEntry->dwUpperBaseAddress = (QWORD) pvHandler >> 32;
	pstEntry->dwReserved = 0;
}

// IDT 테이블 초기화 함수
void kInitializeIDTTables(void){
	IDTR* pstIDTR;
	IDTENTRY* pstEntry;
	int i;

	// IDTR 시작 어드레스
	pstIDTR = (IDTR*) IDTR_STARTADDRESS;
	// IDT테이블 정보 생성
	pstEntry = (IDTENTRY*) (IDTR_STARTADDRESS + sizeof(IDTR));
	pstIDTR->qwBaseAddress = (QWORD) pstEntry;
	pstIDTR->wLimit = IDT_TABLESIZE - 1;

	// 100개의 벡터를 전부 DummyHandler로 연결
	for(i = 0; i < IDT_MAXENTRYCOUNT; i++)
		kSetIDTEntry(&(pstEntry[i]), kDummyHandler, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
}

// 임시 핸들러
void kDummyHandler(void){
	kPrintString(0, 0, "================================================================");
	kPrintString(0, 1, "                   Dummy Interrupt Handler Execute              ");
	kPrintString(0, 2, "                    Interrupt or Exception Occur                ");
	kPrintString(0, 3, "================================================================");

	while(1);
}
