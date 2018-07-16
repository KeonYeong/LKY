#include "Types.h"
#include "Page.h"
#include "ModeSwitch.h"

void kPrintString( int iX, int iY, const char* pcString );
BOOL kInitializeKernel64Area(void);
BOOL kIsMemoryEnough(void);
void kCopyKernel64ImageTo2Mbyte(void);

void Main( void )
{

    DWORD i;
	DWORD dwEAX, dwEBX, dwECX, dwEDX;
	char vcVendorString[13] = {0, };

	kPrintString( 0, 3, "Protected Mode C Language Kernel started by LKY..........[Done]" );

	// 최소 메모리 크기 만족 여부 검사
    kPrintString(0, 4, "Minimum Memory Size Check..........[    ]");
    if(kIsMemoryEnough() == FALSE)
    {
        kPrintString(36, 4, "Fail");
        kPrintString(0, 5, "[ERROR]Not Enough Memory! LKY OS Requires more than 64Mbyte Memory");
        while(1);
    }
    else
    {
        kPrintString(36, 4, "Done");
    }
    
	// IA-32e 모드의 커널 영역 초기화
    kPrintString(0, 5, "IA-32e Kernel Area Initialize..........[    ]");
    if(kInitializeKernel64Area() == FALSE)
    {
        kPrintString(40, 5, "Fail");
        kPrintString(0, 6, "[ERROR]Kernel Area Initialization Fail!");
        while(1);
    }
    kPrintString(40, 5, "Done");

	// IA-32e 모드 커널을 위한 페이지 테이블 생성
	kPrintString(0, 6, "IA-32e Page Tables Initialize..........[    ]");
	kInitializePageTables();
	kPrintString(40, 6, "Done");

	// 프로세스 제조사 정보 읽은 후에 64비트 지원 유무를 확인하는 부분
	kReadCPUID(0x00, &dwEAX, &dwEBX, &dwECX, &dwEDX); // 0x00로 CPUID를 실행 후 뒤의 인자들에게 결과 값을 저장
	*(DWORD*)vcVendorString = dwEBX;
	*((DWORD*)vcVendorString + 1) = dwEDX;
	*((DWORD*)vcVendorString + 2) = dwECX;
	// 제조사 정보 출력
	kPrintString(0, 7, "Processor Vendor String..........[            ]");
	kPrintString(34, 7, vcVendorString);
	// 64비트 지원 유무 확인
	kReadCPUID(0x80000001, &dwEAX, &dwEBX, &dwECX, &dwEDX);
	kPrintString(0, 8, "64bit Mode Support Check..........[    ]");
	if (dwEDX & (1 << 29))
		kPrintString(35, 8, "Done");
	else {
		kPrintString(35, 8, "Fail");
		kPrintString(0, 9, "This Processor doesn't support 64bit mode!");
		while(1);
	}
	
	// IA-32e 커널 이미지를 0x200000로 이동
	kPrintString(0, 9, "Copy IA-32e Kernel To 2M Address..........[    ]");
	kCopyKernel64ImageTo2Mbyte();
	kPrintString(43, 9, "Done");
	// IA-32e 모드 전환
	kPrintString(0, 9, "Switch To IA-32e Mode");
	kSwitchAndExecute64bitKernel();

	while(1);
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

BOOL kInitializeKernel64Area(void)
{
    DWORD* pdwCurrentAddress;

    pdwCurrentAddress = (DWORD*) 0x100000;

    while((DWORD)pdwCurrentAddress < 0x600000)
    {
        *pdwCurrentAddress = 0x00;

        if(*pdwCurrentAddress != 0)
        {
            return FALSE;
        }

        pdwCurrentAddress++;
    }

    return TRUE;
}

BOOL kIsMemoryEnough(void)
{
    DWORD* pdwCurrentAddress;

    pdwCurrentAddress = (DWORD*) 0x100000;

    while((DWORD)pdwCurrentAddress < 0x4000000)
    {
        *pdwCurrentAddress = 0x12345678;

        if(*pdwCurrentAddress != 0x12345678)
        {
            return FALSE;
        }

        pdwCurrentAddress += (0x100000 / 4);
    }

    return TRUE;
}

void kCopyKernel64ImageTo2Mbyte(void){
	WORD wKernel32SectorCount, wTotalKernelSectorCount;
	DWORD* pdwSourceAddress, * pdwDestinationAddress;
	int i;

	wTotalKernelSectorCount = *((WORD*) 0x7C05);
	wKernel32SectorCount = *((WORD*) 0x7C07);

	pdwSourceAddress = (DWORD*) (0x10000 + (wKernel32SectorCount * 512));
	pdwDestinationAddress = (DWORD*) 0x200000;

	for(i = 0 ; i < 512 * (wTotalKernelSectorCount - wKernel32SectorCount) / 4; i++){
		*pdwDestinationAddress = *pdwSourceAddress;
		pdwDestinationAddress++;
		pdwSourceAddress++;
	}
}
