#include "PIT.h"

// PIT 초기화 함수
void kInitializePIT(WORD wCount, BOOL bPeriodic){
	// PIT 컨트롤 레지스터(포트 0x43)이고 여기에 값을 초기화함, 카운트 멈춤 후 모드 0, 바이너리 카운터 설정!
	kOutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_ONCE);

	// 만약 주기를 계속 반복하는 거면 모드 2
	if(bPeriodic == TRUE)
		kOutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_PERIODIC);

	// LSB -> MSB 순으로 카운터 0에다가 초기값 설정 (몇초에 한번 인터럽트 발생할 지 설정하는 것)
	kOutPortByte(PIT_PORT_COUNTER0, wCount);
	kOutPortByte(PIT_PORT_COUNTER0, wCount >> 8);
}

// 카운터 0의 현재값 반환하기
WORD kReadCounter0(void){
	BYTE bHighByte, bLowByte;
	WORD wTemp = 0;

	// 래치 커맨드를 컨트롤 레지스터에 전송하면, 읽게 되는 것 그리고 마찬가지로 LSB -> MSB 순으로 데이터 레지스터에서 읽어옴
	kOutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_LATCH);

	bLowByte = kInPortByte(PIT_PORT_COUNTER0);
	bHighByte = kInPortByte(PIT_PORT_COUNTER0);

	wTemp = (bHighByte << 8) | bLowByte;
	return wTemp;
}

// 카운터 0을 직접 설정해서 일정 시간 대기하기
// 이걸 호출 하면 다시 PIT를 초기화해서 재설정해야한다, 또 이거 하기 전 인터럽트 비활성화 하는 게 좀 더 정확하다
void kWaitUsingDirectPIT(WORD wCount){
	WORD wLastCounter0;
	WORD wCurrentCounter0;

	// 0 ~ 0xFFFF까지 반복카운팅하도록 설정
	kInitializePIT(0, TRUE);

	wLastCounter0 = kReadCounter0();
	while(1){
		wCurrentCounter0 = kReadCounter0();
		if(((wLastCounter0 - wCurrentCounter0) & 0xFFFF) >= wCount) break;
	}
}
