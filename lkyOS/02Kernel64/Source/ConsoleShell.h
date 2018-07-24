#ifndef __CONSOLESHELL_H__
#define __CONSOLESHELL_H__

#include "Types.h"

// 매크로
#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT 300
#define CONSOLESHELL_PROMPTMESSAGE "LEEKY>>"

// 문자열 포인터를 파라미터로 받는 함수 포인터
typedef void (*CommandFunction) (const char* pcParameter);

// 구조체
#pragma pack(push, 1)

// 셀의 커맨드 관리 자료구조
typedef struct kShellCommandEntryStruct{
	char* pcCommand;
	char* pcHelp;
	CommandFunction pfFunction;
}SHELLCOMMANDENTRY;

// 파라미터 처리를 위한 자료구조
typedef struct kParameterListStruct{
	const char* pcBuffer;
	int iLength;
	int iCurrentPosition;
}PARAMETERLIST;

#pragma pack(pop)

// 함수
// 실제 shell 코드
void kStartConsoleShell(void);
void kExecuteCommand(const char* pcCommandBuffer);
void kInitializeParameter(PARAMETERLIST* pstList, const char* pcParameter);
int kGetNextParameter(PARAMETERLIST* pstList, char* pcParameter);

// 커맨드 함수
void kHelp(const char* pcParameterBuffer);
void kCls(const char* pcParameterBuffer);
void kShowTotalRAMSize(const char* pcParameterBuffer);
void kStringToDecimalHexTest(const char* pcParameterBuffer);
void kShutdown(const char* pcParameterBuffer);
void kSetTimer(const char* pcParameterBuffer);
void kWaitUsingPIT(const char* pcParameterBuffer);
void kReadTimeStampCounter(const char* pcParameterBuffer);
void kMeasureProcessorSpeed(const char* pcParameterBuffer);
void kShowDateAndTime(const char* pcParameterBuffer);
void kCreateTestTask(const char* pcParameterBuffer);

#endif /*__CONSOLESHELL_H__*/
