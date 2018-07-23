#ifndef __PIT_H__
#define __PIT_H__

#include "Types.h"

// 매크로
#define PIT_FREQUENCY 1193182 // 클록 주파수임
#define MSTOCOUNT(x) (PIT_FREQUENCY * (x) / 1000) // 초당 위 숫자만큼 뛰는 거니까 몇번 뛸 지 정해서 원하는 초만큼 걸리게 하고 거기서 1000을 나눔으로써 밀리세컨드로 바꾼다
#define USTOCOUNT(x) (PIT_FREQUENCY * (x) / 1000000) // 마찬가지, 다만 1000000로 나눠서 마이크로세컨드로 바꾼다

// I/O 포트
#define PIT_PORT_CONTROL 0x43
#define PIT_PORT_COUNTER0 0x40
#define PIT_PORT_COUNTER1 0x41
#define PIT_PORT_COUNTER2 0x42

// 모드
#define PIT_CONTROL_COUNTER0 0x00
#define PIT_CONTROL_COUNTER1 0x40
#define PIT_CONTROL_COUNTER2 0x80
#define PIT_CONTROL_LSBMSBRW 0x30
#define PIT_CONTROL_LATCH 0x00
#define PIT_CONTROL_MODE0 0x00
#define PIT_CONTROL_MODE2 0x04

// Binary or BCD
#define PIT_CONTROL_BINARYCOUNTER 0x00
#define PIT_CONTROL_BCDCOUNTER 0x01

// 조합
#define PIT_COUNTER0_ONCE (PIT_CONTROL_COUNTER0 | PIT_CONTROL_LSBMSBRW | PIT_CONTROL_MODE0 | PIT_CONTROL_BINARYCOUNTER)
#define PIT_COUNTER0_PERIODIC (PIT_CONTROL_COUNTER0 | PIT_CONTROL_LSBMSBRW | PIT_CONTROL_MODE2 | PIT_CONTROL_BINARYCOUNTER)
#define PIT_COUNTER0_LATCH (PIT_CONTROL_COUNTER0 | PIT_CONTROL_LATCH)

// 함수
void kInitializePIT(WORD wCount, BOOL bPeriodic);
WORD kReadCounter0(void);
void kWaitUsingDirectPIT(WORD wCount);

#endif /*__PIT_H__*/

