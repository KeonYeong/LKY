#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"

// 커맨드 테이블 정의하기, 커맨드 테이블에서 문자열 비교후 함수 포인터를 사용해서 해당 커맨드를 실행할 것이다
SHELLCOMMANDENTRY gs_vstCommandTable[] =
{
	{"help", "Show Help", kHelp},
	{"cls", "Clear Screen", kCls},
	{"totalram", "Show Total RAM Size", kShowTotalRAMSize},
	{"strtod", "String To Decimal/Hex Convert", kStringToDecimalHexTest},
	{"shutdown", "Shutdown And Reboot OS", kShutdown},
};

//======================
// 이제 실제 shell 코드
//======================

// 메인 루프
void kStartConsoleShell(void){
	char vcCommandBuffer[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
	int iCommandBufferIndex = 0;
	BYTE bKey;
	int iCursorX, iCursorY;

	// 프롬프트 출력 (키 입력 준비 완료 표시)
	kPrintf(CONSOLESHELL_PROMPTMESSAGE);

	while(1){
		// 키 수신 대기
		bKey = kGetCh();

		// Backspace (지우기) 처리
		if(bKey == KEY_BACKSPACE){
			// 커맨드 버퍼가 차있을 경우에만 지우기가 가능하므로 조건 처리한다
			if(iCommandBufferIndex > 0){
				// 커서 위치 앞으로 옮겨서 지우고 커맨드 버퍼에서 마지막 문자 지우기
				kGetCursor(&iCursorX, &iCursorY);
				kPrintStringXY(iCursorX -1, iCursorY, " ");
				kSetCursor(iCursorX -1, iCursorY);
				iCommandBufferIndex--;
			}
		}

		// 엔터 키 처리
		else if(bKey == KEY_ENTER){
			kPrintf("\n");
			
			// 커맨드 버퍼가 차있으면 해당 커맨드를 실행한다!
			if(iCommandBufferIndex > 0){
				vcCommandBuffer[iCommandBufferIndex] = '\0';
				kExecuteCommand(vcCommandBuffer);
			}

			// 프롬프트 다시 출력하고 커맨드 버퍼는 초기화 한다
			kPrintf(CONSOLESHELL_PROMPTMESSAGE);
			kMemSet(vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
			iCommandBufferIndex = 0;
		}


		// Shift 등 나머지 특수키는 일단 무시
		else if((bKey == KEY_LSHIFT) || (bKey == KEY_RSHIFT) || (bKey == KEY_CAPSLOCK) || (bKey == KEY_NUMLOCK) || (bKey == KEY_SCROLLLOCK)) ;

		// 일반 문자 처리
		else {
			// TAB 일 경우, 커맨드 버퍼에는 공백으로 들어갈 것이기에 공백 처리
			if(bKey == KEY_TAB) bKey = ' ';

			// 버퍼에 공간이 있어야만 커맨드 버퍼에 문자 추가
			if(iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT){
				vcCommandBuffer[iCommandBufferIndex++] = bKey;
				kPrintf("%c", bKey);
			}
			else kPrintf("\nToo Long Command, Press Enter First");
		}
	}
}

// 커맨드 버퍼에 있는 커맨드를 테이블과 비교해서 해당 함수 수행하는 함수
void kExecuteCommand(const char* pcCommandBuffer){
	int i, iSpaceIndex;
	int iCommandBufferLength, iCommandLength;
	int iCount;

	// 공백으로 커맨드 분리하여 추출함 (첫번째가 커맨드이기에 첫째 공백까지만 탐색)
	iCommandBufferLength = kStrLen(pcCommandBuffer);
	for(iSpaceIndex = 0; iSpaceIndex < iCommandBufferLength; iSpaceIndex++)
		if(pcCommandBuffer[iSpaceIndex] == ' ') break;

	// 커맨드 테이블에서 찾아보기
	iCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);
	for(i = 0; i < iCount; i ++){
		iCommandLength = kStrLen(gs_vstCommandTable[i].pcCommand);

		// 길이, 내용까지 다 일치하는 지 확인하여 일치하면 해당 함수를 실행!
		if((iCommandLength == iSpaceIndex) && (kMemCmp(gs_vstCommandTable[i].pcCommand, pcCommandBuffer, iSpaceIndex) == 0)){
			gs_vstCommandTable[i].pfFunction(pcCommandBuffer + iSpaceIndex + 1); // 인자 넘기고 함수 실행
			break;
		}
	}

	// 테이블에 없으면 에러
	if(i >= iCount)
		kPrintf("%s\n", pcCommandBuffer);
}

//  파라미터 자료구조 초기화 하기, 여러개 파라미터는 공백으로 구분지어서 하나의 문자열로 주게 된다
void kInitializeParameter(PARAMETERLIST* pstList, const char* pcParameter){
	pstList->pcBuffer = pcParameter;
	pstList->iLength = kStrLen(pcParameter);
	pstList->iCurrentPosition = 0;
}

// 다음 파라미터 가져오기
int kGetNextParameter(PARAMETERLIST* pstList, char* pcParameter){
	int i;
	int iLength;

	// 파라미터가 없으면 걍 나감
	if(pstList->iLength <= pstList->iCurrentPosition) return 0;

	// 공백을 찾아서 해당 위치까지 파라미터를 복사하고, 현 위치를 업데이트 시켜주며 찾은 파라미터의 길이를 구해 반환한다, 복사해논 파라미터의 끝은 널문자를 추가한다
	for(i = pstList->iCurrentPosition; i < pstList->iLength; i ++){
		if(pstList->pcBuffer[i] == ' ') break;
	}
	kMemCpy(pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i);
	iLength = i - pstList->iCurrentPosition;
	pcParameter[iLength] = '\0';

	pstList->iCurrentPosition += iLength + 1;

	return iLength;
}

//====================
// 커맨드 처리 함수들
//====================

// 도움말 출력 함수
void kHelp(const char* pcCommandBuffer){
	int i;
	int iCount;
	int iCursorX, iCursorY;
	int iLength, iMaxCommandLength = 0;

	kPrintf("===================================================================\n");
	kPrintf("                         Shell Guide                               \n");
	kPrintf("===================================================================\n");

	iCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);

	// 가장 긴 커맨드 길이 계산, 이건 보기 좋은 포맷을 가지게 하기 위해 공백 처리 하려고
	for(i = 0; i < iCount; i++){
		iLength = kStrLen(gs_vstCommandTable[i].pcCommand);
		if(iLength > iMaxCommandLength)
			iMaxCommandLength = iLength;
	}

	// 도움말 출력 하기
	for(i = 0; i < iCount; i ++){
		kPrintf(gs_vstCommandTable[i].pcCommand);
		kGetCursor(&iCursorX, &iCursorY);
		kSetCursor(iMaxCommandLength, iCursorY);
		kPrintf(" - %s\n", gs_vstCommandTable[i].pcHelp);
	}
}

// 화면 지우기
void kCls(const char* pcParameterBuffer){
	kClearScreen();
	// 맨 위는 인터럽트 표시용이라 비워둔다
	kSetCursor(0, 1);
}

// 메모리 총 크기 출력
void kShowTotalRAMSize(const char* pcParameterBuffer){
	kPrintf("Total RAM Size is : %d MB\n", kGetTotalRAMSize());
}

// 숫자문자열을 숫자로 변환해서 출력함
void kStringToDecimalHexTest(const char* pcParameterBuffer){
	char vcParameter[100];
	int iLength;
	PARAMETERLIST stList;
	int iCount = 0;
	long lValue;

	// 파라미터 초기화
	kInitializeParameter(&stList, pcParameterBuffer);

	while(1){
		iLength = kGetNextParameter(&stList, vcParameter);

		// 만약 다음 파라미터 길이가 0으로 나오면 없는거니까 종료
		if(iLength == 0) break;

		kPrintf("PARAM %d = '%s', Length = %d, ", iCount + 1, vcParameter, iLength);

		// 0x로 시작하면 16진수로 바꾸고, 아니면 모두 10진수로 바꿔서 출력함
		if(kMemCmp(vcParameter, "0x", 2) == 0){
			lValue = kAToI(vcParameter + 2, 16);
			kPrintf("HEX Value = %q\n", lValue);
		}

		else{
			lValue = kAToI(vcParameter, 10);
			kPrintf("Decimal Value = %d\n", lValue);
		}
		iCount ++;
	}
}

// PC 재시작
void kShutdown(const char* pcParameterBuffer){
	kPrintf("System Shutdown Start...\n");

	// 키보드 아무거나 눌러서 PC 재시작함
	kPrintf("Press Any Key To Reboot PC...");
	kGetCh();
	kReboot();
}
