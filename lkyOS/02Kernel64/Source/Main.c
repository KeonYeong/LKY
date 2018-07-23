#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"

void kPrintString( int iX, int iY, const char* pcString );

void Main( void )
{
	int iCursorX, iCursorY;
	
	// 콘솔 초기화를 통해 일단 커서 초기화
	kInitializeConsole(0, 10);
	char vcTemp[2] = {0, };
	BYTE bFlags;
	BYTE bTemp;
	int i =0;
	KEYDATA stData;

	kPrintf("Switch To IA-32e Mode Success\n");
	kPrintf("IA-32e C Language Kernel Start..............[Done]\n");

	// GDTR 새로 설정하여 디스크립터 설정을 바꾸고 TSS 세그먼트 또한 설정
	kGetCursor(&iCursorX, &iCursorY);
	kPrintf("GDT Initialize And Switch For IA-32e Mode...[    ]");
	kInitializeGDTTableAndTSS();
	kLoadGDTR(GDTR_STARTADDRESS);
	kSetCursor(45, iCursorY++);
	kPrintf("Done\n");

	kPrintf("TSS Segment Load............................[    ]");
	kLoadTR(GDT_TSSSEGMENT);
	kSetCursor(45, iCursorY++);
	kPrintf("Done\n");

	// IDTR 설정하는 부분
	kPrintf("IDT Initialize..............................[    ]");
	kInitializeIDTTables();
	kLoadIDTR(IDTR_STARTADDRESS);
	kSetCursor(45, iCursorY++);
	kPrintf("Done\n");

	// 유효 램 체크
	kPrintf("Total RAM Size Check........................[    ]");
	kCheckTotalRAMSize();
	kSetCursor(45, iCursorY++);
	kPrintf("Done], Size = %d MB\n", kGetTotalRAMSize());
	
	//키보드 활성화
	kPrintf("Keyboard Activate...........................[    ]");
	if(kInitializeKeyboard() == TRUE) {
		kSetCursor(45, iCursorY++);
		kPrintf("Done\n");
		kChangeKeyboardLED(FALSE, FALSE, FALSE);
	}
	else {
		kSetCursor(45, iCursorY++);
		kLKYPrintf(ERRORMESSAGE, "Fail\n");
		while(1);
	}

	// PIC 컨트롤러 초기화, 후에 모든 인터럽트 활성화(마스킹과 EnableInterrupt를 통해 수행)
	kPrintf("PIC Controller And Interrupt Initialize.....[    ]");
	kInitializePIC();
	kMaskPICInterrupt(0); // 모든 값을 0으로 줌으로써 마스킹을 해제하고 인터럽트를 수신한다
	kEnableInterrupt();
	kSetCursor(45, iCursorY++);
	kPrintf("Done\n");

	// 간단한 커널, 셀 시작
	kStartConsoleShell();
}
