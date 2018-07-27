#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"
#include "Task.h"
#include "ShutDownPC.h"

// 커맨드 테이블 정의하기, 커맨드 테이블에서 문자열 비교후 함수 포인터를 사용해서 해당 커맨드를 실행할 것이다
SHELLCOMMANDENTRY gs_vstCommandTable[] =
{
	{"help", "Show Help", kHelp},
	{"cls", "Clear Screen", kCls},
	{"totalram", "Show Total RAM Size", kShowTotalRAMSize},
	{"strtod", "String To Decimal/Hex Convert", kStringToDecimalHexTest},
	{"shutdown", "Shutdown And Reboot OS", kShutdown},
	{"settimer", "Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)", kSetTimer},
	{"wait", "Wait ms Using PIT, ex)wait 100(ms)", kWaitUsingPIT},
	{"rdtsc", "Read Time Stamp Counter", kReadTimeStampCounter},
	{"cpuspeed", "Measure Processor Speed", kMeasureProcessorSpeed},
	{"date", "Show Date And Time", kShowDateAndTime},
	{"createtask", "Create Task, ex)createtask 1(type) 10(count)", kCreateTestTask},
	{"changepriority", "Change Task Priority, ex)changepriority 1(ID) 2(Priority)", kChangeTaskPriority},
	{"tasklist", "Show Task List", kShowTaskList},
	{"killtask", "End Task, ex)killtask 1(ID)", kKillTask},
	{"cpuload", "Show Processor Load", kCPULoad},
	{"poweroff", "Shutdown PC", kPowerOffPC},
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
	kLKYPrintf(PROMPTMESSAGE, CONSOLESHELL_PROMPTMESSAGE);

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
			kLKYPrintf(PROMPTMESSAGE, CONSOLESHELL_PROMPTMESSAGE);
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
				kLKYPrintf(INPUTMESSAGE ,"%c", bKey);
			}
			else kLKYPrintf(ERRORMESSAGE, "\nToo Long Command, Press Enter First");
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
		kLKYPrintf(ERRORMESSAGE, "%s not defined\n", pcCommandBuffer);
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
static void kHelp(const char* pcCommandBuffer){
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
static void kCls(const char* pcParameterBuffer){
	kClearScreen();
	// 맨 위는 인터럽트 표시용이라 비워둔다
	kSetCursor(0, 1);
}

// 메모리 총 크기 출력
static void kShowTotalRAMSize(const char* pcParameterBuffer){
	kPrintf("Total RAM Size is : %d MB\n", kGetTotalRAMSize());
}

// 숫자문자열을 숫자로 변환해서 출력함
static void kStringToDecimalHexTest(const char* pcParameterBuffer){
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
static void kShutdown(const char* pcParameterBuffer){
	kPrintf("System Shutdown Start...\n");

	// 키보드 아무거나 눌러서 PC 재시작함
	kPrintf("Press Any Key To Reboot PC...");
	kGetCh();
	kReboot();
}

// 타이머 시작!(Counter0 사용)
static void kSetTimer(const char* pcParameterBuffer){
	char vcParameter[100];
	PARAMETERLIST stList;
	long lValue;
	BOOL bPeriodic;

	// 파라미터 초기화
	kInitializeParameter(&stList, pcParameterBuffer);

	// ms 추출
	if(kGetNextParameter(&stList, vcParameter) == 0){
		kLKYPrintf(ERRORMESSAGE, "ex)settimer 10(ms) 1(periodic)\n");
		return;
	}
	lValue = kAToI(vcParameter, 10);
	if(kMemCmp(vcParameter, "0x", 2) == 0) lValue = kAToI(vcParameter + 2, 16);
	else lValue = kAToI(vcParameter, 10);

	// Periodic 추출
	if(kGetNextParameter(&stList, vcParameter) == 0){
		kLKYPrintf(ERRORMESSAGE, "ex)settimer 10(ms) 1(periodic)\n");
		return;
	}
	bPeriodic = kAToI(vcParameter, 10);

	// 타이머 초기화
	kInitializePIT(MSTOCOUNT(lValue), bPeriodic);
	kPrintf("Time = %d ms, Periodic = %d Change Complete\n", ((WORD)(MSTOCOUNT(lValue) + 1)) * 1000 / PIT_FREQUENCY, bPeriodic);
}

// PIT 컨트롤러 사용 하여 ms 시간동안 대기시키기
static void kWaitUsingPIT(const char* pcParameterBuffer){
	char vcParameter[100];
	int iLength;
	PARAMETERLIST stList;
	long lMillisecond;
	int i;

	// 파라미터 초기화
	kInitializeParameter(&stList, pcParameterBuffer);
	if(kGetNextParameter(&stList, vcParameter) == 0){
		kLKYPrintf(ERRORMESSAGE, "ex)wait 100(ms)\n");
		return;
	}

	// 밀리세컨드 추출
	lMillisecond = kAToI(vcParameter, 10);
	kPrintf("%d ms Sleep start\n", lMillisecond);
	// 인터럽트 비활성화 후 30ms 단위로 여러본 wait(30)을 호출시키고, 30으로 나눈 나머지만큼 한번 더 wait시킴
	kDisableInterrupt();
	for(i = 0; i < lMillisecond / 30; i ++) kWaitUsingDirectPIT(MSTOCOUNT(30));
	kWaitUsingDirectPIT(MSTOCOUNT(lMillisecond % 30));
	kEnableInterrupt();
	kPrintf("%d ms Sleep Complete\n", lMillisecond);

	// 타이머 복원
	kInitializePIT(MSTOCOUNT(1), TRUE);
}

// 타임 스탬프 카운터 읽는 함수
static void kReadTimeStampCounter(const char* pcParameterBuffer){
	QWORD qwTSC;

	qwTSC = kReadTSC();
	kPrintf("Time Stamp Counter = %q\n", qwTSC);
}

// 프로세서 속도 측정 함수
static void kMeasureProcessorSpeed(const char* pcParameterBuffer){
	int i;
	QWORD qwLastTSC, qwTotalTSC = 0;

	kPrintf("Now Measuring.");

	kDisableInterrupt();
	// 10초 동안 변화한 타임 스탬프 카운터를 사용하여 프로세서 속도 측정함
	for(i = 0; i < 200; i ++){
		qwLastTSC = kReadTSC();
		kWaitUsingDirectPIT(MSTOCOUNT(50));
		qwTotalTSC += kReadTSC() - qwLastTSC;

		kPrintf(".");
	}

	// 타이머 복원
	kInitializePIT(MSTOCOUNT(1), TRUE);
	kEnableInterrupt();

	kPrintf("\nCPU Speed = %d MHz\n", qwTotalTSC / 10 / 1000 / 1000);
}

// RTC컨트롤러로 저아된 일자, 시간 표시
static void kShowDateAndTime(const char* pcParameterBuffer){
	BYTE bSecond, bMinute, bHour;
	BYTE bDayOfWeek, bDayOfMonth, bMonth;
	WORD wYear;

	// 읽어 오고
	kReadRTCTime(&bHour, &bMinute, &bSecond);
	kReadRTCDate(&wYear, &bMonth, &bDayOfMonth, &bDayOfWeek);

	kPrintf("Date: %d/%d/%d %s, ", wYear, bMonth, bDayOfMonth, kConvertDayOfWeekToString(bDayOfWeek));
	kPrintf("Time: %d:%d:%d\n", bHour, bMinute, bSecond);
}

// 태스크 테스트 :: 전환 함수
static void kTestTask1(void){
	BYTE bData = 0;
	int i = 0, iX = 0, iY = 0, iMargin, j;
	CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;
	TCB* pstRunningTask;

	pstRunningTask = kGetRunningTask();
	iMargin = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) % 10; 

	for(j = 0; j < 20000; j ++)
	{
		switch(i) 
		{
			case 0:
				iX++;
				if(iX >= (CONSOLE_WIDTH - iMargin)) i = 1;
				break;

			case 1:
				iY++;
				if(iY >= (CONSOLE_HEIGHT - iMargin)) i = 2;
				break;

			case 2:
				iX--;
				if(iX < iMargin) i = 3;
				break;

			case 3:
				iY--;
				if(iY < iMargin) i = 0;
				break;
		}
		pstScreen[iY * CONSOLE_WIDTH + iX].bCharactor = bData;
		pstScreen[iY * CONSOLE_WIDTH + iX].bAttribute = bData & 0x0F;
		bData++;

		//kSchedule();
	}   
	kExitTask();
}

static void kTestTask2( void )
{
	int i = 0, iOffset;
	CHARACTER* pstScreen = (CHARACTER*)CONSOLE_VIDEOMEMORYADDRESS;
	TCB* pstRunningTask;
	char vcData[4] = {'-', '\\', '|', '/'};

	pstRunningTask = kGetRunningTask();
	iOffset = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) * 2;
	iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

	while( 1 )
	{
		pstScreen[iOffset].bCharactor = vcData[i % 4];
		pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;
		i++;

		//kSchedule();
	}
}


// 태스크 테스트 :: 생성 함수
static void kCreateTestTask(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcType[30];
	char vcCount[30];
	int i;

	kInitializeParameter(&stList, pcParameterBuffer);
	kGetNextParameter(&stList, vcType);
	kGetNextParameter(&stList, vcCount);

	switch(kAToI(vcType, 10)) 
	{   
		case 1:
			for(i = 0; i < kAToI(vcCount, 10) ; i ++)
				if(kCreateTask(TASK_FLAGS_LOW, (QWORD)kTestTask1) == NULL) break;
			kPrintf("Task1 %d Created\n", i);
			break;

		case 2:
		default:
			for(i = 0; i < kAToI(vcCount, 10) ; i++)
				if( kCreateTask(TASK_FLAGS_LOW, (QWORD) kTestTask2) == NULL) break;
			kPrintf( "Task2 %d Created\n", i );
			break;
	}
}   

// 태스크 우선순위 변경
static void kChangeTaskPriority(const char* pcParameterBuffer){
	PARAMETERLIST stList;
	char vcID[30];
	char vcPriority[30];
	QWORD qwID;
	BYTE bPriority;

	// 파라미터 추출
	kInitializeParameter(&stList, pcParameterBuffer);
	kGetNextParameter(&stList, vcID);
	kGetNextParameter(&stList, vcPriority);

	// 우선순위 변경
	if(kMemCmp(vcID, "0x", 2) == 0)qwID = kAToI(vcID + 2, 16);
	else qwID = kAToI(vcID, 10);
	bPriority = kAToI(vcPriority, 10);

	kPrintf("Change Task Priority ID [0x%q] Priority[%d]", qwID, bPriority);
	if(kChangePriority(qwID, bPriority) == TRUE)kPrintf("Success\n");
	else kPrintf("Fail\n");
}

// 태스크 정보 출력
static void kShowTaskList(const char* pcParameterBuffer){
	int i;
	TCB* pstTCB;
	int iCount = 0;

	kPrintf("==========Task Total Count[%d]============\n", kGetTaskCount());
	for(i = 0; i < TASK_MAXCOUNT; i ++){
		// TCB를 구해서 사용중이면 ID 출력
		pstTCB = kGetTCBInTCBPool(i);
		if((pstTCB->stLink.qwID >> 32) != 0){
			// 10 개 출력 될때마다 정보 표시 여부 확인
			if((iCount != 0) && ((iCount % 10) == 0)){
				kPrintf("Press Any key to continue ('q' is exit) :");
				if(kGetCh() == 'q'){
					kPrintf("\n");
					break;
				}
				kPrintf("\n");
			}
			kPrintf("[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q]\n", 1 + iCount++, pstTCB->stLink.qwID, GETPRIORITY(pstTCB->qwFlags), pstTCB->qwFlags);
		}
	}
}

// 태스크 종료
static void kKillTask(const char* pcParameterBuffer){
	PARAMETERLIST stList;
	char vcID[30];
	QWORD qwID;

	// 파라미터 추출
	kInitializeParameter(&stList, pcParameterBuffer);
	kGetNextParameter(&stList, vcID);

	// 태스크 종료
	if(kMemCmp(vcID, "0x", 2) == 0) qwID = kAToI(vcID + 2, 16);
	else qwID = kAToI(vcID, 10);

	kPrintf("Kill Task ID [0x%q] : ", qwID);
	if(kEndTask(qwID) == TRUE) kPrintf("Success\n");
	else kPrintf("Fail\n");
}

// 프로세서의 사용률을 표시
static void kCPULoad(const char* pcParameterBuffer){
	kPrintf("Processor Load : %d%%\n", kGetProcessorLoad());
}

static void kPowerOffPC(const char* pcParameterBuffer){
	kShutDownPC();
}
