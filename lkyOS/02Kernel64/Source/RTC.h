#ifndef __RTC_H__
#define __RTC_H__

#include "Types.h"

// 매크로

// I/O 포트
#define RTC_CMOSADDRESS 0x70
#define RTC_CMOSDATA 0x71

// CMOS에 각각 시간 레지스터 어드레스
#define RTC_ADDRESS_SECOND 0x00
#define RTC_ADDRESS_MINUTE 0x02
#define RTC_ADDRESS_HOUR 0x04
#define RTC_ADDRESS_DAYOFWEEK 0x06
#define RTC_ADDRESS_DAYOFMONTH 0x07
#define RTC_ADDRESS_MONTH 0x08
#define RTC_ADDRESS_YEAR 0x09

// BCD 를 Binary로 바꾸는 매크로, 한 숫자가 무조건 4비트로 표현되는 것이 BCD이기 때문에, 4비트단위로 끊어서 한 숫자로 변환시켜준다
#define RTC_BCDTOBINARY(x) ((((x) >> 4) * 10) + ((x) & 0x0F))

// 함수
void kReadRTCTime(BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond);
void kReadRTCDate(WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, BYTE* pbDayOfWeek);
char* kConvertDayOfWeekToString(BYTE bDayOfWeek);

#endif /*__RTC_H__*/

