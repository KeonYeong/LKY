#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"

void kPrintString( int iX, int iY, const char* pcString );

void Main( void )
{
	char vcTemp[2] = {0, };
	BYTE bFlags;
	BYTE bTemp;
	int i =0;

	kPrintString(0, 10, "Switch To IA-32e Mode Success");
	kPrintString(0, 11, "IA-32e C Language Kernel Start..............[Done]");
	// GDTR 새로 설정하여 디스크립터 설정을 바꾸고 TSS 세그먼트 또한 설정
	kPrintString(0, 12, "GDT Initialize And Switch For IA-32e Mode...[    ]");
	kInitializeGDTTableAndTSS();
	kLoadGDTR(GDTR_STARTADDRESS);
	kPrintString(45, 12, "Done");
	kPrintString(0, 13, "TSS Segment Load............................[    ]");
	kLoadTR(GDT_TSSSEGMENT);
	kPrintString(45, 13, "Done");
	// IDTR 설정하는 부분
	kPrintString(0, 14, "IDT Initialize..............................[    ]");
	kInitializeIDTTables();
	kLoadIDTR(IDTR_STARTADDRESS);
	kPrintString(45, 14, "Done");
	kPrintString(0, 15, "Keyboard Activate..........[    ]");


	//키보드 활성화
	if(kActivateKeyboard() == TRUE){
		kPrintString(28, 15, "Done");
		kChangeKeyboardLED(FALSE, FALSE, FALSE);
	}
	else
	{
		kPrintString(28, 15, "Fail");
		while(1);
	}


	while(1){
		// 출력 버퍼가 차 있으면 코드 읽을 수 있는 것
		if(kIsOutputBufferFull() == TRUE){
			bTemp = kGetKeyboardScanCode();

			if(kConvertScanCodeToASCIICode(bTemp, &(vcTemp[0]), &bFlags) == TRUE)
				if(bFlags & KEY_FLAGS_DOWN)
				{
					kPrintString(i++, 16, &(vcTemp[0]));
					if(vcTemp[0] == '0')
						bTemp = bTemp / 0;
				}
		}
	}
}

void kPrintString( int iX, int iY, const char* pcString)
{
    CHARACTER* pstScreen = ( CHARACTER* ) 0xB8000;
    int i;

    pstScreen += ( iY * 80 ) + iX;

    for( i = 0 ; pcString[i] != 0 ; i ++)
    {
        pstScreen[i].bCharactor = pcString[i];
    }
}
