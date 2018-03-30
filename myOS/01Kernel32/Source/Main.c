#include "Types.h"

void kPrintString( int iX, int iY, const char* pcString );
BOOL kInitializeKernel64Area(void);
BOOL kIsMemoryEnough(void);

void Main( void )
{

    DWORD i;

    kPrintString( 0, 3, "C Kernel started by LKY..........[Done]" );

    kPrintString(0, 4, "Minimum Memory Size Check..........[    ]");
    if(kIsMemoryEnough() == FALSE)
    {
        kPrintString(36, 4, "Fail");
        kPrintString(0, 5, "[ERROR]Not Enough Memory! DF OS Requires more than 64Mbyte Memory");
        while(1);
    }
    else
    {
        kPrintString(36, 4, "Done");
    }
    
    kPrintString(0, 5, "IA-32e Kernel Area Initialize..........[    ]");
    if(kInitializeKernel64Area() == FALSE)
    {
        kPrintString(40, 5, "Fail");
        kPrintString(0, 6, "[ERROR]Kernel Area Initialization Fail!");
        while(1);
    }
    kPrintString(40, 5, "Done");

    while (1) ;
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
