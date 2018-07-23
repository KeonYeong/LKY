#include "PIC.h"

// PIC 초기화 함수
void kInitializePIC(void){
	// 우선 마스터 PIC 초기화
	kOutPortByte(PIC_MASTER_PORT1, 0x11); // 우선 0x20포트에 ICW1
	kOutPortByte(PIC_MASTER_PORT2, PIC_IRQSTARTVECTOR); // 후 0x20부터(32) 벡터
	kOutPortByte(PIC_MASTER_PORT2, 0x04); // 2번 핀에 슬레이브
	kOutPortByte(PIC_MASTER_PORT2, 0x01); // EOI 전송 모드, 8086모드

	// 슬레이브 PIC
	kOutPortByte(PIC_SLAVE_PORT1, 0x11);
	kOutPortByte(PIC_SLAVE_PORT2, PIC_IRQSTARTVECTOR + 8); // 0x20 뒤 8번째 후 슬레이브임 (40)
	kOutPortByte(PIC_SLAVE_PORT2, 0x02); // 마스터와 달리 몇번째인지 숫자로 바로 씀
	kOutPortByte(PIC_SLAVE_PORT2, 0x01); 
}

// Interrupt Masking 함수
void kMaskPICInterrupt(WORD wIRQBitmask){ // 2바이트로 비트 마스킹을 인자로 받음
	// 마스터 PIC의 IMR을 설정하는데, 두번째 포트에 하위 8비트만 쓰면 됨
	kOutPortByte(PIC_MASTER_PORT2, (BYTE) wIRQBitmask);

	// 이제 슬레이브
	kOutPortByte(PIC_SLAVE_PORT2, (BYTE) (wIRQBitmask >> 8));
}

// EOI (End Of Interrupt) 보내는 함수
void kSendEOIToPIC(int iIRQNumber){
	// 마스터 두번째 포트에 0110,0x60 (특정 핀넘버로) 혹은 0010,0x20 (최상위 인터럽트)를 보냄
	// 슬레이브 interrupt일 경우에만 슬레이브에게도 보냄
	kOutPortByte(PIC_MASTER_PORT1, 0x20);

	if(iIRQNumber >= 8)
		kOutPortByte(PIC_SLAVE_PORT1, 0x20);
}


