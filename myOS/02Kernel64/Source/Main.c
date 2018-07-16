#include "Types.h"
#include "Keyboard.h"

void kPrintString( int iX, int iY, const char* pcString );

void Main( void )
{
	char vcTemp[2] = {0, };
	BYTE bFlags;
	BYTE bTemp;
	int i =0;

	kPrintString(0, 10, "Switch To IA-32e Mode Success");
	kPrintString(0, 11, "IA-32e C Language Kernel Start..............[Done]");
	kPrintString(0, 12, "Keyboard Activate..........[    ]");

	//키보드 활성화
	if(kActivateKeyboard() == TRUE){
		kPrintString(28, 12, "Done");
		kChangeKeyboardLED(FALSE, FALSE, FALSE);
	}
	else
	{
		kPrintString(28, 12, "Fail");
		while(1);
	}

	while(1){
		// 출력 버퍼가 차 있으면 코드 읽을 수 있는 것
		if(kIsOutputBufferFull() == TRUE){
			bTemp = kGetKeyboardScanCode();

			if(kConvertScanCodeToASCIICode(bTemp, &(vcTemp[0]), &bFlags) == TRUE)
				if(bFlags & KEY_FLAGS_DOWN)
					kPrintString(i++, 13, &(vcTemp[0]));
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
