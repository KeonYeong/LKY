#include "Utility.h"
#include "AssemblyUtility.h"
#include <stdarg.h>

// 메모리 특정 값으로 채우는 것
void kMemSet(void* pvDestination, BYTE bData, int iSize){
	int i;
	for (i = 0; i < iSize; i++)
		((char*)pvDestination)[i] = bData;
}

// 메모리 복사
int kMemCpy(void* pvDestination, const void* pvSource, int iSize){
	int i;
	for (i = 0; i < iSize; i++)
		((char*)pvDestination)[i] = ((char*)pvSource)[i];
	return iSize;
}

// 메모리 비교
int kMemCmp(const void* pvDestination, const void* pvSource, int iSize){
	int i;
	char cTemp;

	for(i = 0; i < iSize; i ++){
		cTemp = ((char*)pvDestination)[i] - ((char*)pvSource)[i];
		if(cTemp != 0)
			return (int)cTemp;
	}
	return 0;
}

// RFLAGS 인터럽트 플래그 체크하고 반환하며 RFLAGS를 갱신함
BOOL kSetInterruptFlag(BOOL bEnableInterrupt){
	QWORD qwRFLAGS;

	// 이전 걸 읽음 후에 처리함
	qwRFLAGS = kReadRFLAGS();
	if(bEnableInterrupt == TRUE) kEnableInterrupt();
	else kDisableInterrupt();

	// 이전 RFLAGS의 플래그 반환
	if(qwRFLAGS & 0x0200) return TRUE;

	return FALSE;
}

// atoi() 함수 구현
long kAToI(const char* pcBuffer, int iRadix){
	long lReturn;
	switch(iRadix){
		// 16진수
		case 16:
			lReturn = kHexStringToQword(pcBuffer);
			break;

		// 10진수, 기타
		case 10:
		default:
			lReturn = kDecimalStringToLong(pcBuffer);
			break;
	}
	return lReturn;
}

// atoi()용 16진수 문자열을 Qword로 반환하기
QWORD kHexStringToQword(const char* pcBuffer){
	QWORD qwValue = 0;
	int i;

	// 문자열 돌면서 다 반환하기
	for(i = 0; pcBuffer[i] != '\0'; i++){
		// 자릿수 늘어났으니까 16곱해놓고 밑에 자리 구하기
		qwValue *= 16;

		// 위에건 A ~ F 처리, 밑에건 그냥 일반 숫자 처리
		if(('A' <= pcBuffer[i]) && ('Z' >= pcBuffer[i])) qwValue += (pcBuffer[i] - 'A') + 10;
		else if (('a' <= pcBuffer[i]) && ('z' >= pcBuffer[i])) qwValue += (pcBuffer[i] - 'a') + 10;
		else qwValue += pcBuffer[i] - '0';
	}
	return qwValue;
}

// atoi()용 10진수 문자열 long으로 반환하기
long kDecimalStringToLong(const char* pcBuffer){
	long lValue = 0;
	int i;

	// 음수면 -를 제외하고 먼저 나머지를 long으로 변환
	if (pcBuffer[0] == '-') i = 1;
	else i = 0;

	// 16진수와 마찬가지
	for( ; pcBuffer[i] != '\0'; i++){
		// 자릿수 증가
		lValue *= 10;
		lValue += pcBuffer[i] - '0';
	}
	
	// 음수일 경우 - 추가하기
	if(pcBuffer[0] == '-') lValue = -lValue;

	return lValue;
}

// itoa() 함수 구현
int kIToA(long lValue, char* pcBuffer, int iRadix){
	int iReturn;

	switch(iRadix){
		// 16진수
		case 16:
			iReturn = kHexToString(lValue, pcBuffer);
			break;

		// 10, 기타
		case 10:
		default:
			iReturn = kDecimalToString(lValue, pcBuffer);
	}
	// 반환값은 문자열 길이이다
	return iReturn;
}

// itoa()용 16진수 숫자 문자열로 변환
int kHexToString(QWORD qwValue, char* pcBuffer){
	QWORD i;
	QWORD qwCurrentValue;

	// 0은 바로 처리해버림
	if(qwValue == 0){
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}

	// 버퍼에 1의 자리를 시작으로 해서 숫자 삽입
	for(i = 0; qwValue > 0; i ++){
		qwCurrentValue = qwValue % 16;
		if(qwCurrentValue >= 10) pcBuffer[i] = 'A' + (qwCurrentValue - 10); // A ~ F 일 경우 처리
		else pcBuffer[i] = '0' + qwCurrentValue; // 나머지
		// 이제 qwValue를 16으로 나눠서 한자리 위 숫자를 확인함
		qwValue = qwValue / 16;
	}

	// 마지막엔 항상 널문자!
	pcBuffer[i] = '\0';

	// 버퍼에는 1의자리가 맨 앞에 들어가있기 때문에, 거꾸로 뒤집어서 반환한다
	kReverseString(pcBuffer);
	return i;
}

int kDecimalToString(long lValue, char* pcBuffer){
	long i;
	// 0 처리
	if(lValue == 0){
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}

	// 음수면 일단 -를 추가해놓고 양수로 변환시키고 출력하기
	if(lValue < 0){
		i = 1;
		pcBuffer[0] = '-';
		lValue = -lValue;
	}
	else i = 0;

	// 본격 변환, 16진수랑 비슷하다
	for (; lValue > 0; i++){
		pcBuffer[i] = '0' + lValue % 10;
		lValue = lValue / 10;
	}

	// 널문자
	pcBuffer[i] = '\0';

	// 마찬가지로 뒤집는데, 음수일 경우 부호 제외하고 뒤집는다
	if(pcBuffer[0] == '-') kReverseString(&(pcBuffer[1]));
	else kReverseString(pcBuffer);

	return i;
}

// 문자열 뒤집기
void kReverseString(char* pcBuffer){
	int iLength;
	int i;
	char cTemp;

	// 문자열 가운데를 중심으로 잡고 좌/우를 바꾸는 거
	iLength = kStrLen(pcBuffer);
	for(i = 0 ; i < iLength / 2; i++){ // 홀수, 짝수 신경 안 써도 된다
		cTemp = pcBuffer[i];
		pcBuffer[i] = pcBuffer[iLength - 1 - i];
		pcBuffer[iLength - 1 - i] = cTemp;
	}
}

// 문자열 길이 반환 함수
int kStrLen(const char* pcBuffer){
	int i;

	for(i = 0; ; i++)
		if(pcBuffer[i] == '\0') break;

	return i;
}

// sprintf() 함수 구현하기!, 가변인자 함수
int kSPrintf(char* pcBuffer, const char* pcFormatString, ...){
	va_list ap;
	int iReturn;

	// 가변인자를 ap에 넣는다, 매크로를 사용하게 되는데 간단히 앞 인자의 주소를 찾아가서 그곳이 스택의 주소이므로 거기서 8을 더하여 가변리스트의 시작 주소로 잡는다, va_list는 결국 int 형임
	va_start(ap, pcFormatString);
	iReturn = kVSPrintf(pcBuffer, pcFormatString, ap);
	// 가변인자 사용 종료 매크로 (아마도 주소 지정을 해제시키는 것 같다)
	va_end(ap);

	return iReturn;
}

// vsprintf() 함수 구현하기! 가변인자와 출력 형태를 받아서 문자열로 출력버퍼에 복사하는 것
int kVSPrintf(char* pcBuffer, const char* pcFormatString, va_list ap){
	QWORD i, j;
	int iBufferIndex = 0;
	int iFormatLength, iCopyLength;
	char* pcCopyString;
	QWORD qwValue;
	int iValue;

	// 포맷 문자열 길이를 읽기
	iFormatLength = kStrLen(pcFormatString);
	// 그 후 해당 길이만큼 데이터를 출력 버퍼에 복사하기
	for(i = 0; i < iFormatLength; i ++){
		// %로 시작할 경우 데이터 타입 문자인 것!
		if(pcFormatString[i] == '%'){
			// % 다음 문자를 본다
			i++;
			switch(pcFormatString[i]){ // 다음 문자가 s, d, f 등 여러가지인데 그에 따라 맞게끔 문자열로 변환하고 출력 버퍼로 복사 한 후에 출력 버퍼 인덱스는 하나 늘려준다, 가변인자 리스트에서 하나씩 빼오면서 수행하면 된다
				case 's': // 문자열일 경우!
					pcCopyString = (char*) (va_arg(ap, char*)); // va_arg는 가변인자 첫번째 변수를 가져오는 건데, 데이터 타입을 지정해주면서 스택의 바로 위에 있는 변수를 바로 참조시키는 것이다
					iCopyLength = kStrLen(pcCopyString);
					// 문자열 길이만큼 복사하고, 버퍼 인덱스도 바꾼다
					kMemCpy(pcBuffer + iBufferIndex, pcCopyString, iCopyLength);
					iBufferIndex += iCopyLength;
					break;

				case 'c': // 문자일 경우! 하나만 바꿔서 바로 복사하고, 인덱스 변경
					pcBuffer[iBufferIndex] = (char) (va_arg(ap, int)); // 스택 하나의 크기는 char이 아닌 int 따라서 int 만큼 가져옴
					iBufferIndex++;
					break;

				case 'd': // 정수일 경우!
				case 'i':
					iValue = (int) (va_arg(ap, int));
					iBufferIndex += kIToA(iValue, pcBuffer + iBufferIndex, 10); // 인덱스는 늘려주고, itoa()로 동시에 pcBuffer에다가 해당 숫자를 복사함
					break;

				case 'x': // 4바이트 hex출력일 경우
				case 'X':
					qwValue = (DWORD) (va_arg(ap, DWORD)) & 0xFFFFFFFF; // 4바이트니까 4바이트만큼만 &연산하여 가져온다
					iBufferIndex += kIToA(qwValue, pcBuffer + iBufferIndex, 16);
					break;

				case 'q': // 8바이트 hex출력일 경우
				case 'Q':
				case 'p':
					qwValue = (QWORD) (va_arg(ap, QWORD));
					iBufferIndex += kIToA(qwValue, pcBuffer + iBufferIndex, 16);
					break;

				default: // 아무것도 해당하지 않을 경우 그냥 그대로 문자만 출력 한다(% 없이)
					pcBuffer[iBufferIndex] = pcFormatString[i];
					iBufferIndex++;
					break;
			}
		}

		// %가 아닌 일반 문자열 처리, 그냥 넣고 인덱스 올리고 다음으로 넘어감
		else{
			pcBuffer[iBufferIndex] = pcFormatString[i];
			iBufferIndex++;
		}
	}

	// 마지막은 항상 널문자 추가
	pcBuffer[iBufferIndex] = '\0';
	return iBufferIndex; // 문자열의 길이 반환
}

// shell 커맨드 함수를 위한 함수, 램 크기 체크 함수
static gs_qwTotalRAMMBSize = 0; // 램 총 크기
// 최초 부팅 과정에서 한번 호출 하는 것
void kCheckTotalRAMSize(void){
	DWORD* pdwCurrentAddress;
	DWORD dwPreviousValue;

	// 64MB(0x4000000) 부터 4MB 단위로 검사 시작
	pdwCurrentAddress = (DWORD*) 0x4000000;
	while(1){
		dwPreviousValue = *pdwCurrentAddress;
		// 값을 써봤는데 만약 그 값이 제대로 안써지면 유효하지 않은 메모리임
		*pdwCurrentAddress = 0x12345678;
		if(*pdwCurrentAddress != 0x12345678) break;
		
		// 이전 값 복원 하고 다음 위치로
		*pdwCurrentAddress = dwPreviousValue;
		pdwCurrentAddress += (0x400000 / 4);
	}
	// 체크 성공한 어드레스는 MB단위로 계산하여 저장
	gs_qwTotalRAMMBSize = (QWORD) pdwCurrentAddress / 0x100000;
}
// RAM크기 반환
QWORD kGetTotalRAMSize(void){
	return gs_qwTotalRAMMBSize;
}


