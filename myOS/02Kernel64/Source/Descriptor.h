#ifndef __DESCRIPTOR_H__
#define __DESCRIPTOR_H__

#include "Types.h"

// GDT===================
// 조합에 쓰려는 매크로
#define GDT_TYPE_CODE 0x0A
#define GDT_TYPE_DATA 0x02
#define GDT_TYPE_TSS 0x09
#define GDT_FLAGS_LOWER_S 0x10
#define GDT_FLAGS_LOWER_DPL0 0x00
#define GDT_FLAGS_LOWER_DPL1 0x20
#define GDT_FLAGS_LOWER_DPL2 0x40
#define GDT_FLAGS_LOWER_DPL3 0x60
#define GDT_FLAGS_LOWER_P 0x80
#define GDT_FLAGS_UPPER_L 0x20
#define GDT_FLAGS_UPPER_DB 0x40
#define GDT_FLAGS_UPPER_G 0x80

// 실제 사용하는 매크로
// Lower는 Code,Data,TSS와 DPL0와 Present로 설정
#define GDT_FLAGS_LOWER_KERNELCODE (GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P) // 커널 코드 디스크립터
#define GDT_FLAGS_LOWER_KERNELDATA (GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P) // 커널 데이터 디스크립터
#define GDT_FLAGS_LOWER_TSS (GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P) // TSS
#define GDT_FLAGS_LOWER_USERCODE (GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P) // 유저 코드 디스크립터
#define GDT_FLAGS_LOWER_USERDATA (GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P) // 유저 데이터 디스크립터

//Upper는 Granulaty로 설정, 코드 와 데이터는 64비트 추가
#define GDT_FLAGS_UPPER_CODE (GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L)
#define GDT_FLAGS_UPPER_DATA (GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L)
#define GDT_FLAGS_UPPER_TSS (GDT_FLAGS_UPPER_G)

// 세그먼트 디스크립터 오프셋
#define GDT_KERNELCODESEGMENT 0x08
#define GDT_KERNELDATASEGMENT 0x10
#define GDT_TSSSEGMENT 0x18

// 기타 GDT 관련 매크로
#define GDTR_STARTADDRESS 0x142000 // GDTR 시작 어드레스
#define GDT_MAXENTRY8COUNT 3 // 8바이트 엔트리 개수(널, 코드, 데이터)
#define GDT_MAXENTRY16COUNT 1 // 16바이트 엔트리 개수 (TSS)
#define GDT_TABLESIZE ((sizeof(GDTENTRY8) * GDT_MAXENTRY8COUNT) + (sizeof(GDTENTRY16) * GDT_MAXENTRY16COUNT)) // GDT 테이블 크기
#define TSS_SEGMENTSIZE (sizeof(TSSSEGMENT)) // TSS 세그먼트 크기

// IDT===================
// 조합에 쓰려는 매크로
#define IDT_TYPE_INTERRUPT 0x0E
#define IDT_TYPE_TRAP 0x0F
#define IDT_FLAGS_DPL0 0x00
#define IDT_FLAGS_DPL1 0x20
#define IDT_FLAGS_DPL2 0x40
#define IDT_FLAGS_DPL3 0x60
#define IDT_FLAGS_P 0x80
#define IDT_FLAGS_IST0 0
#define IDT_FLAGS_IST1 1

// 실제 사용하는 매크로
#define IDT_FLAGS_KERNEL (IDT_FLAGS_DPL0 | IDT_FLAGS_P)
#define IDT_FLAGS_USER (IDT_FLAGS_DPL3 | IDT_FLAGS_P)

// 기타 IDT 관련 매크로
#define IDT_MAXENTRYCOUNT 100 // IDT 엔트리 개수
#define IDTR_STARTADDRESS (GDTR_STARTADDRESS + sizeof(GDTR) + GDT_TABLESIZE + TSS_SEGMENTSIZE) // IDTR 시작 어드레스, GDT + TSS 다음
#define IDT_STARTADDRESS (IDTR_STARTADDRESS + sizeof(IDTR)) // IDT 테이블 시작 어드레스
#define IDT_TABLESIZE (IDT_MAXENTRYCOUNT * sizeof(IDTENTRY)) // IDT 테이블 크기

// IST 관련, 시작 어드레스와 크기
#define IST_STARTADDRESS 0x700000
#define IST_SIZE 0x100000

// 구조체
// 1바이트로 정렬
#pragma pack(push, 1)
// GDT=============
// GDTR과 IDTR 구조체
typedef struct kGDTRStruct{
	WORD wLimit;
	QWORD qwBaseAddress;
	// 16바이트 정렬 위해 추가
	WORD wPading;
	DWORD dwPading;
} GDTR, IDTR;

// 8바이트의 GDT 엔트리 구조
typedef struct kGDTEntry8Struct{ // 8바이트는 코드와 데이터 세그먼트 디스크립터를 위한 것
	WORD wLowerLimit;
	WORD wLowerBaseAddress;
	BYTE bUpperBaseAddress1;
	// 4비트 Type, 1비트 S, 2비트 DPL, 1비트 P
	BYTE bTypeAndLowerFlag;
	// 4비트 Segment Limit, 1비트 AVL, L, D/B, G
	BYTE bUpperLimitAndUpperFlag;
	BYTE bUpperBaseAddress2;
}GDTENTRY8;

// 16바이트의 GDT 엔트리 구조
typedef struct kGDTEntry16Struct{ // 16바이트는 TSS를 위한 것
	WORD wLowerLimit;
	WORD wLowerBaseAddress;
	BYTE bMiddleBaseAddress1;
	// 4비트 Type, 1비트 0, 2비트 DPL, 1비트 P
	BYTE bTypeAndLowerFlag;
	// 4비트 Segment Limit, 1비트 AVL, 0, 0, G
	BYTE bUpperLimitAndUpperFlag;
	BYTE bMiddleBaseAddress2;
	DWORD dwUpperBaseAddress;
	DWORD dwReserved;
}GDTENTRY16;

// TSS Data 구조체
typedef struct kTSSDataStruct{
	DWORD dwReserved1;
	QWORD qwRsp[3];
	QWORD qwReserved2;
	QWORD qwIST[7];
	QWORD qwReserved3;
	WORD wReserved;
	WORD wIOMapBaseAddress;
}TSSSEGMENT;

// IDT=============
typedef struct kIDTEntryStruct{
	WORD wLowerBaseAddress;
	WORD wSegmentSelector;
	BYTE bIST; // 3비트 IST, 5비트 0
	BYTE bTypeAndFlags; // 4비트 Type, 1비트 0, 2비트 DPL, 1비트 P
	WORD wMiddleBaseAddress;
	DWORD dwUpperBaseAddress;
	DWORD dwReserved;
} IDTENTRY;

#pragma pack(pop)

// 함수
void kSetGDTEntry8(GDTENTRY8* pstEntry, DWORD dwBaseAddress, DWORD dwLimit, BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType);
void kSetGDTEntry16(GDTENTRY16* pstEntry, QWORD qwBaseAddress, DWORD dwLimit, BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType);
void kInitializeGDTTableAndTSS(void);
void kInitializeTSSSegment(TSSSEGMENT* pstTSS);

void kSetIDTEntry(IDTENTRY* pstEntry, void* pvHandler, WORD wSelector, BYTE bIST, BYTE bFlags, BYTE bType);
void kDummyHandler(void);
void kInitializeIDTTables(void);

#endif
