#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <stdarg.h>
#include "Types.h"

// 함수
void kMemSet(void* pvDestination, BYTE bData, int iSize);
int kMemCpy(void* pvDestination, const void* pvSource, int iSize);
int kMemCmp(const void* pvDestination, const void* pvSource, int iSize);
BOOL kSetInterruptFlag(BOOL bEnableInterrupt);
long kAToI(const char* pcBuffer, int iRadix);
QWORD kHexStringToQword(const char* pcBuffer);
long kDecimalStringToLong(const char* pcBuffer);
int kIToA(long lValue, char* pcBuffer, int iRadix);
int kHexToString(QWORD qwValue, char* pcBuffer);
int kDecimalToString(long lValue, char* pcBuffer);
void kReverseString(char* pcBuffer);
int kStrLen(const char* pcBuffer);
int kSPrintf(char* pcBuffer, const char* pcFormatString, ...);
int kVSPrintf(char* pcBuffer, const char* pcFormatString, va_list ap);
void kCheckTotalRAMSize(void);
QWORD kGetTotalRAMSize(void);

#endif /*__UTILITY_H__*/
