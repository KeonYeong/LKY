#include <stdarg.h>
#include "Console.h"
#include "Keyboard.h"

// 콘솔 정보 관리하는 자료구조!
CONSOLEMANAGER gs_stConsoleManager = {0, };

// 콘솔 초기화 함수
void kInitializeConsole(int iX, int iY){
	//  0으로
	kMemSet(&gs_stConsoleManager, 0, sizeof(gs_stConsoleManager));

	// 커서 위치 설정
	kSetCursor(iX, iY);
}

// 커서 위치 설정 함수, 출력 포트를 사용하며, 별도의 컨트롤러 내부의 커서 관리 기능을 사용한다
void kSetCursor(int iX, int iY){
	int iLinearValue; // 해당 컨트롤러는 column, line 을 안 따지고 바로 offset으로 관리하기에 offset으로 바꿔줘야한다, 해당 변수

	// 커서 위치 계산
	iLinearValue = iY * CONSOLE_WIDTH + iX;

	// CRTC컨트롤러를 사용하는데, 상위와 하위 위치 레지스터에 각각 상위, 하위 바이트를 전송하여 커서를 출력하는 것이다
	// 포트는 0x3D4를 사용하며, 해당 포트에 0x0E를 전송 시 상위 어드레스 레지스터 선택, 0x0F 전송 시 하위 어드레스 레지스터 선택이다
	// 또한 데이터 레지스터가 존재하는데 해당 레지스터의 포트는 0x3D5이며 여기에 값을 입력하여 커서의 위치를 설정한다
	
	// 상위 커서 어드레스 설정
	kOutPortByte(VGA_PORT_INDEX, VGA_INDEX_UPPERCURSOR);
	// 그 후 데이터 입력
	kOutPortByte(VGA_PORT_DATA, iLinearValue >> 8);

	// 하위 커서 어드레스 설정 후 데이터 입력
	kOutPortByte(VGA_PORT_INDEX, VGA_INDEX_LOWERCURSOR);
	kOutPortByte(VGA_PORT_DATA, iLinearValue & 0xFF);

	// 문자 출력할 위치 업데이트 하기
	gs_stConsoleManager.iCurrentPrintOffset = iLinearValue;
}

// 현 커서 위치 반환 함수
void kGetCursor(int* piX, int* piY){
	// 매니저에 오프셋으로 저장 되있기에 너비로 나누고, 나머지를 통해 x와 y의 값을 얻을 수 있음
	*piX = gs_stConsoleManager.iCurrentPrintOffset % CONSOLE_WIDTH;
	*piY = gs_stConsoleManager.iCurrentPrintOffset / CONSOLE_WIDTH;
}

// printf 함수 구현하기!
void kPrintf(const char* pcFormatString, ...){
	va_list ap;
	char vcBuffer[1024];
	int iNextPrintOffset;

	va_start(ap, pcFormatString);
	kVSPrintf(vcBuffer, pcFormatString, ap); // vsprintf() 사용하여 우선 문자열을 만들어낸다
	va_end(ap);

	// 출력하기!
	iNextPrintOffset = kConsolePrintString(DEFAULTMESSAGE, vcBuffer);

	// 커서 위치 업데이트!
	kSetCursor(iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH);
}

// 나만의 printf 함수 구현
void kLKYPrintf(int iType, const char* pcFormatString, ...){
	va_list ap;
	char vcBuffer[1024];
	int iNextPrintOffset;

	va_start(ap, pcFormatString);
	kVSPrintf(vcBuffer, pcFormatString, ap); // vsprintf() 사용하여 우선 문자열을 만들어낸다
	va_end(ap);

	// 출력하기!
	iNextPrintOffset = kConsolePrintString(iType, vcBuffer);

	// 커서 위치 업데이트!
	kSetCursor(iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH);
}

// printf()를 할 때 \n, \t 또한 처리해줘야 하기 때문에 consoleprintstring을 따로 쓴다, 또한 화면상의 다음 출력 위치를 반환한다
int kConsolePrintString(int iType, const char* pcBuffer){
	CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;
	int i, j;
	int iLength;
	int iPrintOffset;

	//  문자열 출력하는 위치 일단 가져옴
	iPrintOffset = gs_stConsoleManager.iCurrentPrintOffset;

	// 문자열 길이만큼 루프를 돌며 출력한다
	iLength = kStrLen(pcBuffer);
	for(i = 0; i < iLength; i ++){
		// 줄바꿈 \n 처리, 다음 줄로 넘기는데, console의 너비를 채우게끔 더해준다
		if(pcBuffer[i] == '\n') iPrintOffset += (CONSOLE_WIDTH - (iPrintOffset % CONSOLE_WIDTH));

		// 탭 \t 처리, 그냥 8의 배수 위치에 두는것이다, 예로 4번째에 있으면 4만 더해주는 것
		else if(pcBuffer[i] == '\t') iPrintOffset += (8 - (iPrintOffset % 8));

		else { // 일반 문자열 처리하기
			pstScreen[iPrintOffset].bCharactor = pcBuffer[i];
			switch(iType){
				case PROMPTMESSAGE:
					pstScreen[iPrintOffset].bAttribute = LKY_PROMPTTEXTCOLOR;
					break;
				case ERRORMESSAGE:
					pstScreen[iPrintOffset].bAttribute = LKY_ERRORTEXTCOLOR;
					break;
				case INPUTMESSAGE:
					pstScreen[iPrintOffset].bAttribute = LKY_INPUTTEXTCOLOR;
					break;
				case DEFAULTMESSAGE:
				default:
					pstScreen[iPrintOffset].bAttribute = LKY_DEFAULTTEXTCOLOR;
					break;
			}
			iPrintOffset++;
		}

		// 출력하고 나서 위치를 봤을 때 최댓값(너비 * 높이)을 벗어나면 스크롤 처리를 한다, 맨 윗줄 빼고 전부 한 줄씩 올리며 맨 아랫줄은 공백으로 채우는 동작이다
		if(iPrintOffset >= (CONSOLE_HEIGHT * CONSOLE_WIDTH)){
			// 우선 맨 윗줄 제외 전부 복사
			kMemCpy(CONSOLE_VIDEOMEMORYADDRESS, CONSOLE_VIDEOMEMORYADDRESS + CONSOLE_WIDTH * sizeof(CHARACTER), (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH * sizeof(CHARACTER));

			// 마지막 라인 공백 처리
			for(j = (CONSOLE_HEIGHT - 1) * (CONSOLE_WIDTH); j < CONSOLE_HEIGHT * CONSOLE_WIDTH; j++){
				pstScreen[j].bCharactor = ' ';
				pstScreen[j].bAttribute = LKY_DEFAULTTEXTCOLOR;
			}

			// 이제 커서를 마지막 라인 첫번째로 수정한다
			iPrintOffset = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
		}
	}
	// 출력하고 난 다음 오프셋 반환
	return iPrintOffset;
}

// 전체 화면 삭제!
void kClearScreen(void){
	CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;
	int i;

	// 화면 전체를 공백으로 채우기
	for(i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++){
		pstScreen[i].bCharactor = ' ';
		pstScreen[i].bAttribute = LKY_DEFAULTTEXTCOLOR;
	}

	// 커서 화면 상단 이동
	kSetCursor(0, 0);
}

// getch() 함수 구현하여 콘솔 내부에서 하나의 키만 받게 함
BYTE kGetCh(void){
	KEYDATA stData;

	// 키가 눌릴 때까지 대기
	while(1){
		// 키 큐에 데이터가 있을 때까지 기다림
		while(kGetKeyFromKeyQueue(&stData) == FALSE) kSchedule();

		// 플래그를 봐서 키가 눌렸을 경우에만 ASCII 코드 반환
		if(stData.bFlags & KEY_FLAGS_DOWN)
			return stData.bASCIICode;
	}
}

// 문자열 X, Y 에 출력하기
void kPrintStringXY(int iX, int iY, const char* pcString){
	CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;
	int i;

	// 위치 찾고
	pstScreen += (iY * 80) + iX;
	// 길이만큼 출력
	for(i = 0; pcString[i] != 0; i ++){
		pstScreen[i].bCharactor = pcString[i];
		pstScreen[i].bAttribute = LKY_DEFAULTTEXTCOLOR;
	}
}
