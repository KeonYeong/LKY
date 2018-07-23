#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"

// 공통으로 사용하는 예외 핸들러
void kCommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode){
	char vcBuffer[3] = {0,};

	// 화면에 예외 문구 띄우기
	vcBuffer[0] = '0' + iVectorNumber / 10;
	vcBuffer[0] = '0' + iVectorNumber % 10;

	kPrintStringXY(0, 0, "===================================================");
	kPrintStringXY(0, 1, "               Exception Occur! Vector:");
	kPrintStringXY(39, 1, vcBuffer);
	kPrintStringXY(0, 2, "===================================================");

	while(1);
}

// 공통으로 사용하는 인터럽트 핸들러
void kCommonInterruptHandler(int iVectorNumber){
	char vcBuffer[] = "[INT:  , ]";
	static int g_iCommonInterruptCount = 0;

	// 화면 오른쪽 위에 인터럽트 벡터 2자리로 출력하기 + 횟수 출력하기
	vcBuffer[5] = '0' + iVectorNumber / 10;
	vcBuffer[6] = '0' + iVectorNumber % 10;
	vcBuffer[8] = '0' + g_iCommonInterruptCount;
	g_iCommonInterruptCount = (g_iCommonInterruptCount + 1) % 10;
	kPrintStringXY(70, 0, vcBuffer);

	// EOI 전송
	kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR); // 뒤에 있는 매크로는 32이고, 벡터가 32부터 시작하기에 빼준다(핀은 0부터 시작하니까)
}

// 키보드 인터럽트 핸들러
void kKeyboardHandler(int iVectorNumber){
	// 일단은 공통 인터럽트 핸들러와 동일
	char vcBuffer[] = "[INT:  , ]";
	static int g_iKeyboardInterruptCount = 0;
	BYTE bTemp;
	
	// 화면 좌측 위에 2자리로 횟수와 출력하기
	vcBuffer[5] = '0' + iVectorNumber / 10;
	vcBuffer[6] = '0' + iVectorNumber % 10;
	vcBuffer[8] = '0' + g_iKeyboardInterruptCount;
	g_iKeyboardInterruptCount = (g_iKeyboardInterruptCount + 1) % 10;
	kPrintStringXY(0, 0, vcBuffer);

	// 데이터를 읽고 ASCII로 변환해서 삽입
	if(kIsOutputBufferFull() == TRUE){
		bTemp = kGetKeyboardScanCode();
		kConvertScanCodeAndPutQueue(bTemp);
	}

	// EOI 전송
	kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}
