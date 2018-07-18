#ifndef __PIC_H__
#define __PIC_H__

#include "Types.h"

// I/O 포트 정의, 20이 마스터 A0이 슬레이브
#define PIC_MASTER_PORT1 0x20
#define PIC_MASTER_PORT2 0x21
#define PIC_SLAVE_PORT1 0xA0
#define PIC_SLAVE_PORT2 0xA1

// IDT테이블 인터럽트 벡터가 시작되는 위치
#define PIC_IRQSTARTVECTOR 0x20 // 기본적으로 31개까지 예약이 되어있기에 32부터 설정하는 것, 0x20

// 함수
void kInitializePIC(void);
void kMaskPICInterrupt(WORD wIRQBitmask);
void kSendEOIToPIC(int iIRQNumber);

#endif /*__PIC_H__*/
